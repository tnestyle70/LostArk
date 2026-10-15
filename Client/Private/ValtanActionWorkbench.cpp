// Workbench 본체는 도메인 도구의 수명 연결, 읽기 모델 결합, frame 경계와 편집 명령을 조정한다.
// Details와 Preview는 같은 클래스의 다른 CPP이며 별도 draft 소유자를 만들지 않는다.
#include "imgui.h"

#include "ValtanActionWorkbench.h"
#include "CompositionTimeline.h"

#include "GameInstance.h"
#include "Profiler.h"

#include <cstdio>

#include "Animation_Tool.h"
#include "AnimationPreviewAssets.h"
#include "AnimationTargetService.h"
#include "Model.h"
#include "Valtan.h"
#include "BalanceTool.h"
#include "ValtanBossTool.h"
#include "ValtanPatternAuditionService.h"
#include "ValtanPatternFlowService.h"
#include "CameraTool.h"
#include "Effect_Catalog.h"
#include "Effect_Playback.h"
#include "Effect_PresentationService.h"
#include "EffectAuthoringResourceTree.h"
#include "Effect_Tool.h"
#include "EffectV2_Catalog.h"
#include "MainApp.h"
#include "Level_ValtanArena.h"
#include "ProjectDataRoot.h"
#include "ActionPresentationTimeline.h"
#include "RuntimeAssetRoot.h"
#include "SoundCueCatalog.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <functional>
#include <limits>
#include <numeric>
#include <sstream>
#include <string_view>

// Details·Preview에서도 쓰는 보조 함수의 선언을 가져온다. 정의는 아래 이름 있는 private 구현 이름 공간에 한 번만 둔다.
#include "ValtanActionWorkbench_EditSupport.h"

// 이 CPP 안에서 공유 보조 이름을 짧게 참조한다. using은 객체 생성이나 상태 공유를 수행하지 않는다.
using namespace Client::ValtanWorkbenchDetail;

// 익명 이름 공간은 이 CPP만 쓰는 복사·ID·시간 계산 보조 타입과 함수를 외부 링크에서 숨긴다.
// 세션 상태는 클래스 멤버에 남고 여기의 보조 함수는 명시적으로 받은 값이나 기존 소유 도구를 사용한다.
namespace
{

	// 복사·붙여넣기 입력을 값으로 캡처한다. source 행의 주소를 남기면 다음 편집이나 Reload 때 의미가 달라질 수 있다.
	// optional은 선택 종류에 해당하는 payload만 존재함을 표현하며 원본 ID와 revision도 함께 남긴다.
	struct VALTAN_EDIT_TRANSFER final : Client::COMPOSITION_TRANSFER_SNAPSHOT
	{
		std::string_view Type() const noexcept override { return "valtan.occurrence.v1"; }
		Client::CValtanActionWorkbench::DETAIL_OWNER owner{};
		std::string patternId, stageId, sourceRevision, stableId;
		uint32_t startMs = 0u;
		std::optional<Client::CBalanceTool::ANIMATION_SLOT_EDIT> animation;
		std::vector<Client::VALTAN_PRESENTATION_SOURCE_VIEW> animationSources;
		std::optional<Client::CBalanceTool::PATTERN_STAGE_EDIT> collider;
		std::optional<Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW> effect;
		std::optional<Client::EFFECT_V2_BINDING> effectV2;
		std::optional<Client::VALTAN_PATTERN_SOUND_CUE> sound;
		std::optional<Client::BOSS_STAGE_SCENE_PROFILE_OCCURRENCE> scene;
		std::optional<Client::BOSS_STAGE_LIGHT_OCCURRENCE> light;
		std::optional<Client::VALTAN_CAMERA_INVOCATION_VIEW> camera;
		std::optional<Client::VALTAN_COMBAT_OBJECT_EFFECT_VIEW> summon;
		std::vector<Client::VALTAN_STAGE_ACTION_VIEW> actions;
	};
	// 다른 소유 도구에는 공용 Effect 필드를 전달한다. Valtan으로 돌아올 때는 native 복사본도 사용해
	// UI 공용 투영에 없는 정확한 clip·source·종료 정책을 잃지 않는다.
	struct VALTAN_EFFECT_TRANSFER final : Client::COMPOSITION_EFFECT_TRANSFER
	{
		std::shared_ptr<const VALTAN_EDIT_TRANSFER> native;
	};
	struct VALTAN_PRESENTATION_BATCH final : Client::COMPOSITION_TRANSFER_SNAPSHOT
	{
		struct ITEM
		{
			VALTAN_EDIT_TRANSFER value;
			uint32_t offsetMs = 0u;
			double effectDurationWallMs = 0.0;
		};
		std::vector<ITEM> items;
		std::string_view Type() const noexcept override { return "valtan.presentation.batch.v1"; }
	};

	struct VALTAN_STAGE_BATCH final : Client::COMPOSITION_TRANSFER_SNAPSHOT
	{
		struct ITEM
		{
			Client::VALTAN_STAGE_VIEW stage;
			std::vector<Client::VALTAN_PATTERN_SOUND_CUE> sounds;
			std::vector<Client::EFFECT_V2_BINDING> effects;
			std::vector<Client::VALTAN_PATTERN_SHAKE_CUE> shakes;
		};
		std::vector<ITEM> items;
		Client::COMPOSITION_TRANSFER extras;
		uint32_t firstStageMs = 0u, extrasFirstMs = 0u;
		std::string_view Type() const noexcept override { return "valtan.stage.batch.v1"; }
	};

	Client::COMPOSITION_TRANSFER CaptureEffectResource(const std::string& id, const char* kind)
	{
		auto value = std::make_shared<Client::COMPOSITION_EFFECT_TRANSFER>();
		Client::COMPOSITION_EFFECT_ITEM item; item.resourceKind = kind; item.resourceId = id;
		value->label = id; value->items.push_back(std::move(item)); return value;
	}

	const Client::VALTAN_CLIP_OCCURRENCE_VIEW* ResolveEffectAppendClip(
		const Client::VALTAN_STAGE_VIEW& stage, const std::string_view requestedId,
		const std::string_view selectedId, const std::string_view cursorClipId = {})
	{
		const auto findClip = [&stage](const std::string_view id)
			-> const Client::VALTAN_CLIP_OCCURRENCE_VIEW*
		{
			if (id.empty()) return nullptr;
			for (const auto& clip : stage.ClipOccurrences)
				if (clip.strClipOccurrenceId == id) return &clip;
			return nullptr;
		};
		if (const auto clip = findClip(requestedId)) return clip;
		if (const auto clip = findClip(selectedId)) return clip;
		for (const auto& cue : stage.ProductCues)
			if (cue.strOccurrenceId == selectedId || cue.strBindingId == selectedId)
				if (const auto clip = findClip(cue.strClipOccurrenceId)) return clip;
		if (const auto clip = findClip(cursorClipId)) return clip;
		// 화면의 combo는 이후에도 수정할 수 있다. clip이 여러 개라는 이유만으로
		// Effect resource lane을 연 뒤 Append 대상이 계속 미지정 상태가 되지 않게 한다.
		return stage.ClipOccurrences.empty() ? nullptr : &stage.ClipOccurrences.front();
	}

	template<class Items>
	std::string_view ResolveAppendCursorClipId(const Items& items,
		const std::string_view patternId, const std::string_view stageId, const uint32_t cursorMs)
	{
		for (const auto& item : items)
			if (item.eLane == Client::CValtanActionWorkbench::TIMELINE_LANE::ANIMATION &&
				item.strPatternId == patternId && item.strStageId == stageId &&
				item.iStartMs <= cursorMs && cursorMs < item.iEndMs)
				return item.strStableId;
		return {};
	}

	void InsertEffectResourcePaths(Client::COMPOSITION_RESOURCE_TREE_NODE& tree,
		const bool v1, const bool group, const std::vector<std::string>& categories,
		const std::vector<std::vector<std::string>>& patternCategories, const size_t index)
	{
		const auto insert = [&](const std::vector<std::string>& segments)
		{
			std::vector<std::string> path{ v1 ? "V1" : "V2" };
			if (!v1) path.push_back(group ? "Groups" : "Leaves");
			path.insert(path.end(), segments.begin(), segments.end());
			Client::InsertResourceTree(tree, path, index);
		};
		insert(categories);
		for (const auto& path : patternCategories)
			if (path != categories) insert(path);
	}

	// 삭제·재정렬 뒤에도 현재 행과 충돌하지 않는 occurrence ID를 찾는다.
	// 순번 후보마다 기존 행을 순회하므로 크기만으로 즉시 ID를 계산하는 방식보다 검사가 늘지만 실제 중복을 피한다.
	template<class Rows, class Id>
	std::string NextValtanOccurrenceId(const std::string& prefix, const Rows& rows, Id id)
	{
		for (std::size_t ordinal = 1u; ordinal <= rows.size() + 1u; ++ordinal)
		{
			const auto value = prefix + std::to_string(ordinal);
			if (std::none_of(rows.begin(), rows.end(), [&](const auto& row) { return id(row) == value; })) return value;
		}
		return {};
	}

	constexpr float TIMELINE_LANE_LABEL_WIDTH = CompositionTimeline::LabelWidth;
	constexpr float TIMELINE_POINT_MINIMUM_WIDTH_PX = CompositionTimeline::MinimumBoxWidth;
	/* 순간 event에만 최소 표시 폭을 둔다. Effect 행은 방출 지연과 particle 잔여 구간을 포함한 문서 수명을 사용한다. */
	constexpr float SOUND_EVENT_MINIMUM_WIDTH_PX = CompositionTimeline::MinimumBoxWidth;
	constexpr float EFFECT_V2_LEAF_MINIMUM_WIDTH_PX = CompositionTimeline::MinimumBoxWidth;
	constexpr float EFFECT_V2_GROUP_MINIMUM_WIDTH_PX = CompositionTimeline::MinimumBoxWidth;
	constexpr float TIMELINE_CANVAS_END_PADDING_PX = 24.f;
	constexpr double CANONICAL_RELOAD_RETRY_SECONDS = 0.25;
	constexpr float COMPOSITION_LEFT_COLUMN_WIDTH_RATIO = 0.18f;
	constexpr float COMPOSITION_RIGHT_COLUMN_WIDTH_RATIO = 0.20f;
	/* 잘못된 행이나 참조 전용 행 때문에 ImGui 눈금 반복이 무제한 늘어나지 않도록 상한을 둔다.
	   편집 clock에 적용하는 10분 제한을 표시 눈금 계산에도 사용한다. */
	constexpr uint32_t MAX_TIMELINE_RENDER_DURATION_MS = 600000u;

	struct COMPOSITION_DEFAULT_LAYOUT final
	{
		ImVec2 PatternsPos;
		ImVec2 PatternsSize;
		ImVec2 PreviewPos;
		ImVec2 PreviewSize;
		ImVec2 SequencerPos;
		ImVec2 SequencerSize;
		ImVec2 DetailsPos;
		ImVec2 DetailsSize;
		ImVec2 ResourcesPos;
		ImVec2 ResourcesSize;
		ImVec2 SessionPos;
		ImVec2 SessionSize;
	};

	COMPOSITION_DEFAULT_LAYOUT BuildCompositionDefaultLayout(
		const ImGuiViewport* const pViewport)
	{
		const ImVec2 origin = nullptr == pViewport ?
			ImVec2(20.f, 20.f) : pViewport->WorkPos;
		const ImVec2 available = nullptr == pViewport ?
			ImVec2(1600.f, 900.f) : pViewport->WorkSize;
		constexpr float margin = 8.f;
		constexpr float gap = 8.f;
		const float width = (std::max)(available.x, 1.f);
		const float height = (std::max)(available.y, 1.f);
		const float contentWidth = (std::max)(
			1.f, width - margin * 2.f - gap * 2.f);
		const float leftWidth =
			contentWidth * COMPOSITION_LEFT_COLUMN_WIDTH_RATIO;
		const float rightWidth =
			contentWidth * COMPOSITION_RIGHT_COLUMN_WIDTH_RATIO;
		const float centerWidth = contentWidth - leftWidth - rightWidth;
		const float contentHeight = (std::max)(
			1.f, height - margin * 2.f);
		const float leftTopHeight = contentHeight * 0.58f;
		const float leftBottomHeight = (std::max)(
			1.f, contentHeight - leftTopHeight - gap);
		const float centerTopHeight = contentHeight * 0.30f;
		const float sessionHeight = contentHeight * 0.18f;
		const float sequencerHeight = (std::max)(
			1.f, contentHeight - centerTopHeight - sessionHeight - gap * 2.f);

		const float leftX = origin.x + margin;
		const float centerX = leftX + leftWidth + gap;
		const float rightX = centerX + centerWidth + gap;
		const float topY = origin.y + margin;
		COMPOSITION_DEFAULT_LAYOUT layout;
		layout.PatternsPos = ImVec2(leftX, topY);
		layout.PatternsSize = ImVec2(leftWidth, leftTopHeight);
		layout.ResourcesPos = ImVec2(
			leftX, topY + leftTopHeight + gap);
		layout.ResourcesSize = ImVec2(leftWidth, leftBottomHeight);
		layout.PreviewPos = ImVec2(centerX, topY);
		layout.PreviewSize = ImVec2(centerWidth, centerTopHeight);
		layout.SequencerPos = ImVec2(
			centerX, topY + centerTopHeight + gap);
		layout.SequencerSize = ImVec2(centerWidth, sequencerHeight);
		layout.SessionPos = ImVec2(
			centerX, layout.SequencerPos.y + sequencerHeight + gap);
		layout.SessionSize = ImVec2(centerWidth, sessionHeight);
		layout.DetailsPos = ImVec2(rightX, topY);
		layout.DetailsSize = ImVec2(rightWidth, contentHeight);
		return layout;
	}

	std::string Lower(std::string value)
	{
		std::transform(
			value.begin(), value.end(), value.begin(),
			[](const unsigned char character)
			{
				return static_cast<char>(std::tolower(character));
			});
		return value;
	}

	std::string NormalizeBranchStableId(
		const std::string& stageId, const std::string& stableId)
	{
		const std::string prefix = stageId + "/branch/";
		if (0u != stableId.rfind(prefix, 0u))
			return stableId;
		const std::size_t targetSeparator = stableId.find('/', prefix.size());
		return std::string::npos == targetSeparator ?
			stableId : stableId.substr(0u, targetSeparator);
	}

	template <typename T>
	T SaturatingU32(const T value)
	{
		return (std::min)(
			value,
			static_cast<T>((std::numeric_limits<uint32_t>::max)()));
	}

	std::string BuildCompositionSlotId(
		const std::string& patternId,
		const std::string& stageId,
		const std::size_t ordinal)
	{
		std::ostringstream suffix;
		suffix << ".composition.clip." << std::setw(2) << std::setfill('0') <<
			ordinal + 1u;
		const std::string readable = patternId + "." + stageId + suffix.str();
		if (readable.size() <= 160u)
			return readable;
		uint64_t hash = 1469598103934665603ull;
		for (const unsigned char ch : patternId + "/" + stageId)
		{
			hash ^= ch;
			hash *= 1099511628211ull;
		}
		std::ostringstream compact;
		compact << "valtan.composition." << std::hex << std::setw(16) <<
			std::setfill('0') << hash << std::dec << suffix.str();
		return compact.str();
	}

	std::vector<std::string> SplitResourcePath(const std::string_view value)
	{
		std::vector<std::string> Segments;
		std::size_t iStart = 0u;
		for (std::size_t i = 0u; i <= value.size(); ++i)
		{
			const bool_t bBoundary = i == value.size() || '.' == value[i] ||
				'/' == value[i] || ':' == value[i];
			if (!bBoundary)
				continue;
			if (i > iStart)
				Segments.emplace_back(value.substr(iStart, i - iStart));
			iStart = i + 1u;
		}
		return Segments;
	}

	const char_t* EffectV2TypeLabel(const int32_t iType)
	{
		switch (static_cast<Client::EFFECT_V2_TYPE>(iType))
		{
		case Client::EFFECT_V2_TYPE::MESH: return "Mesh";
		case Client::EFFECT_V2_TYPE::TEXTURE: return "Texture";
		case Client::EFFECT_V2_TYPE::PARTICLE: return "Particle";
		case Client::EFFECT_V2_TYPE::DECAL: return "Decal";
		case Client::EFFECT_V2_TYPE::TRAIL: return "Trail";
		case Client::EFFECT_V2_TYPE::SCREEN_POST: return "Screen Post";
		default: return "Unknown";
		}
	}

	float ResolveTimelineDisplayWidthPx(
		const std::string_view strLabel,
		const float fMinimumWidthPx)
	{
		// 다른 Composition 편집기처럼 box 폭은 저작 시각을 따른다.
		// 짧은 cue를 label 길이만큼 늘리지 않고 전체 이름은 hover 상세 정보에 둔다.
		(void)strLabel;
		return (std::max)(fMinimumWidthPx, TIMELINE_POINT_MINIMUM_WIDTH_PX);
	}

	// 각 반복에서 preview와 같은 고정 variation을 선택한다. WAV 길이는 실제 경과 시간 기준이며
	// 원본 animation 재생률은 cue 시작 시각만 바꾼다.
	template<class ResolveDuration>
	bool ResolveValtanSoundTimelineSpan(
		const Client::VALTAN_PATTERN_SOUND_CUE& cue,
		const std::vector<Client::ACTION_PRESENTATION_CLIP_TIMING>& timings,
		const std::size_t clipIndex, const uint32_t stageDurationMs,
		const std::vector<std::string>& variants, ResolveDuration&& resolveDuration,
		uint32_t& startMs, uint32_t& endMs)
	{
		if (variants.empty() || clipIndex >= timings.size() || !stageDurationMs)
			return false;
		const bool repeat = cue.eRepeatPolicy == Client::VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP;
		if (repeat && !timings[clipIndex].bLoop) return false;
		double firstStartMs = -1.0, lastEndMs = 0.0;
		for (uint64_t loop = 0u;; ++loop)
		{
			float sourceWallSeconds = 0.f;
			if (!Client::CActionPresentationTimeline::Resolve_CueWallOffset(timings,
				clipIndex, cue.iStartMs * .001f, loop, sourceWallSeconds)) return false;
			const double triggerMs = sourceWallSeconds * 1000.0;
			if (triggerMs >= stageDurationMs) break;
			if (loop >= 16384u) return false; // 잘못된 반복 입력에는 preview와 같은 반복 횟수 상한을 적용한다.
			const auto occurrenceId = cue.strOccurrenceId + "/loop:" + std::to_string(loop);
			uint64_t selection = 14695981039346656037ull;
			for (const unsigned char c : occurrenceId)
				selection = (selection ^ c) * 1099511628211ull;
			const uint32_t naturalMs = resolveDuration(variants[selection % variants.size()]);
			const uint32_t offsetMs = cue.bHasPlaybackOffset ? cue.iPlaybackOffsetMs : 0u;
			if (naturalMs <= offsetMs) return false;
			const uint32_t durationMs = cue.bHasPlaybackDuration ?
				(std::min)(cue.iPlaybackDurationMs, naturalMs - offsetMs) : naturalMs - offsetMs;
			if (!durationMs) return false;
			if (firstStartMs < 0.0) firstStartMs = triggerMs;
			lastEndMs = (std::max)(lastEndMs, triggerMs + durationMs);
			if (!repeat) break;
		}
		if (firstStartMs < 0.0) return false;
		startMs = static_cast<uint32_t>(std::llround(firstStartMs));
		endMs = static_cast<uint32_t>((std::min)(std::round(lastEndMs),
			static_cast<double>((std::numeric_limits<uint32_t>::max)())));
		return endMs > startMs;
	}

	uint32_t ResolveEffectDocumentSpanMs(const Client::EFFECT_DOCUMENT_DESC& Document)
	{
		double endSeconds = 0.0;
		for (const auto& element : Document.Elements)
		{
			if (!element.bVisible) continue;
			const auto& shape = element.SourceRecipe.strRendererShape;
			const bool sourceParticle = element.SourceRecipe.bEnabled &&
				(shape == "mesh" || shape == "sprite" || shape == "decal");
			endSeconds = (std::max)(endSeconds, static_cast<double>(
				Client::CEffectPlayback::Calculate_ElementEndSeconds(element, sourceParticle)));
		}
		for (const auto& cue : Document.ModelCues)
			if (cue.bVisible) endSeconds = (std::max)(endSeconds,
				static_cast<double>(Client::Effect_ModelCueEndSeconds(cue)));
		for (const auto& control : Document.OwnerControls)
			if (!control.Keys.empty()) endSeconds = (std::max)(endSeconds,
				static_cast<double>(control.fStartSeconds + control.Keys.back().fSeconds));
		return std::isfinite(endSeconds) && endSeconds > 0.0 ?
			static_cast<uint32_t>((std::min)(std::ceil(endSeconds * 1000.0), 600000.0)) : 0u;
	}

    struct EFFECT_V2_TIMELINE_SPAN final
    {
        double fMilliseconds = 0.0;
        bool bResolved = true;
        bool bUnbounded = false;
    };

    // CEffectV2Object와 같은 방출·Deactivate 정책으로 저작 tail의 상한을 계산한다.
    // particle 난수나 이동 trail 표본에 따라 실제 종료는 이 상한보다 빠를 수 있다.
    EFFECT_V2_TIMELINE_SPAN ResolveEffectV2LeafSpanMs(
        const Client::EFFECT_V2_DOCUMENT* document,
        const double stopMs = -1.0,
        const Client::EFFECT_V2_CHILD_STOP stop = Client::EFFECT_V2_CHILD_STOP::KILL)
    {
        if (!document) return {0.0, false, false};
        const auto& params = document->Desc.Params;
        const double rate = params.fPlayRate;
        if (!std::isfinite(rate) || rate <= 0.0 || !std::isfinite(params.fLifetime))
            return {0.0, false, false};
        const double infinity = (std::numeric_limits<double>::infinity)();
        const double emission = !params.bLoop && params.fLifetime > 0.f ?
            double(params.fLifetime) * 1000.0 / rate : infinity;
        double tail = 0.0;
        if (document->eType == Client::EFFECT_V2_TYPE::PARTICLE)
            tail = (std::max)(.001, double(params.Particle.vLifetime.y)) * 1000.0 / rate;
        else if (document->eType == Client::EFFECT_V2_TYPE::TRAIL)
            tail = (std::max)(.001, double(params.Trail.fPointLifetime)) * 1000.0 / rate;
        if (!std::isfinite(tail)) return {0.0, false, false};
        double end = emission + tail;
        if (stopMs >= 0.0)
            end = stop == Client::EFFECT_V2_CHILD_STOP::KILL ?
                (std::min)(end, stopMs) : (std::min)(emission, stopMs) + tail;
        return std::isfinite(end) ? EFFECT_V2_TIMELINE_SPAN{end, true, false} :
            EFFECT_V2_TIMELINE_SPAN{0.0, true, true};
    }

    EFFECT_V2_TIMELINE_SPAN ResolveEffectV2ResourceSpanMs(
        const Client::EFFECT_V2_CATALOG_SNAPSHOT& snapshot,
        const Client::EFFECT_V2_BINDING& binding,
        const double clipStopMs = -1.0)
    {
        if (binding.strGroupId.empty())
            return ResolveEffectV2LeafSpanMs(snapshot.Find_Document(binding.strEffectId), clipStopMs);
        const auto* group = snapshot.Find_Group(binding.strGroupId);
        if (!group) return {0.0, false, false};
        EFFECT_V2_TIMELINE_SPAN result;
        for (const auto& child : group->Children)
        {
            // Catalog 승인은 중첩 group을 거절하므로 여기서 별도의 중첩 실행 규칙을 만들지 않는다.
            if (child.eResourceKind != Client::EFFECT_V2_RESOURCE_KIND::LEAF)
                return {0.0, false, false};
            double stop = child.iDurationMs ? double(child.iStartMs) + child.iDurationMs : -1.0;
            const auto cap = [&stop](const double end)
            { if (end >= 0.0) stop = stop < 0.0 ? end : (std::min)(stop, end); };
            if (group->iDurationMs) cap(group->iDurationMs);
            cap(clipStopMs);
            // 이 child의 생성 시각보다 이른 stop은 허용된 occurrence를 만들 수 없다.
            if (stop >= 0.0 && stop <= child.iStartMs) continue;
            const auto span = ResolveEffectV2LeafSpanMs(
                snapshot.Find_Document(child.strEffectId),
                stop < 0.0 ? -1.0 : stop - child.iStartMs, child.eStop);
            if (!span.bResolved) return span;
            result.bUnbounded |= span.bUnbounded;
            result.fMilliseconds = (std::max)(result.fMilliseconds,
                double(child.iStartMs) + span.fMilliseconds);
        }
        return result;
    }

    uint32_t ResolveEffectV2TimelineEndMs(const EFFECT_V2_TIMELINE_SPAN& span,
        const double startMs, const double rate, const uint32_t stageEndMs,
        const Client::EFFECT_V2_STOP_POLICY stop)
    {
        // 길이를 모르거나 무한한 resource에는 표시 범위만 정한다. 10분짜리 수명을 새로 만든다는 뜻이 아니며
        // 행 label에서 이 대체 표시임을 구분한다.
        double end = span.bResolved && !span.bUnbounded ?
            startMs + span.fMilliseconds / rate : double(stageEndMs);
        if (stop == Client::EFFECT_V2_STOP_POLICY::STAGE_END ||
            stop == Client::EFFECT_V2_STOP_POLICY::CLIP_OCCURRENCE_END)
            end = (std::min)(end, double(stageEndMs));
        return static_cast<uint32_t>((std::clamp)(std::ceil(end), 0.0,
            double((std::numeric_limits<uint32_t>::max)())));
    }

	std::string ClipReplacementRole(const std::string& clip)
	{
		const std::string normalized = Lower(clip);
		if (std::string::npos != normalized.find("_start"))
			return "start";
		if (std::string::npos != normalized.find("_loop"))
			return "loop";
		if (std::string::npos != normalized.find("_end"))
			return "end";
		return {};
	}

	bool_t IsRoleAwareHoldReplacementChain(
		const std::vector<std::string>& Roles)
	{
		if (Roles.size() < 3u || "start" != Roles.front())
			return false;
		std::size_t iRole = 1u;
		const std::size_t iLoopBegin = iRole;
		while (iRole < Roles.size() && "loop" == Roles[iRole])
			++iRole;
		if (iRole == iLoopBegin || iRole >= Roles.size() ||
			"end" != Roles[iRole])
		{
			return false;
		}
		++iRole;
		return std::all_of(
			Roles.begin() + static_cast<std::ptrdiff_t>(iRole), Roles.end(),
			[](const std::string& Role)
			{
				return Role.empty();
			});
	}

	bool_t FitCompositionSequenceCutsToStage(
		const std::vector<uint32_t>& RequestedCutsMs,
		const std::vector<std::string>& Roles,
		const uint32_t iStageDurationMs,
		std::vector<uint32_t>& OutFittedCutsMs,
		std::string& Status)
	{
		OutFittedCutsMs = RequestedCutsMs;
		const uint64_t iRequestedTotalMs = std::accumulate(
			RequestedCutsMs.begin(), RequestedCutsMs.end(), uint64_t{ 0u });
		if (iRequestedTotalMs <= iStageDurationMs)
			return true;
		if (RequestedCutsMs.size() != Roles.size() ||
			!IsRoleAwareHoldReplacementChain(Roles))
		{
			Status = "Replace rejected: this Sequence requires " +
				std::to_string(iRequestedTotalMs) +
				" ms, but the selected Server Stage is " +
				std::to_string(iStageDurationMs) +
				" ms. Only start / one-or-more loop / end / optional untagged tail HOLD chains have a deterministic fit policy.";
			return false;
		}

		std::vector<std::size_t> LoopIndices;
		uint64_t iFixedEdgeMs = 0u;
		uint64_t iRequestedLoopMs = 0u;
		for (std::size_t iClip = 0u; iClip < Roles.size(); ++iClip)
		{
			if ("loop" == Roles[iClip])
			{
				LoopIndices.push_back(iClip);
				iRequestedLoopMs += RequestedCutsMs[iClip];
			}
			else
			{
				iFixedEdgeMs += RequestedCutsMs[iClip];
			}
		}
		if (iFixedEdgeMs + LoopIndices.size() > iStageDurationMs)
		{
			Status = "Replace rejected: fixed start/end/tail clips require " +
				std::to_string(iFixedEdgeMs) + " ms plus " +
				std::to_string(LoopIndices.size()) +
				" positive loop occurrence(s), but the selected Server Stage is " +
				std::to_string(iStageDurationMs) + " ms.";
			return false;
		}

		uint64_t iRemainingBudgetMs = iStageDurationMs - iFixedEdgeMs;
		uint64_t iRemainingRequestedMs = iRequestedLoopMs;
		for (std::size_t iLoop = 0u; iLoop < LoopIndices.size(); ++iLoop)
		{
			const std::size_t iClip = LoopIndices[iLoop];
			const std::size_t iRemainingClipCount =
				LoopIndices.size() - iLoop - 1u;
			uint64_t iAllocationMs = iRemainingBudgetMs;
			if (0u != iRemainingClipCount)
			{
				const uint64_t iProportionalMs = 0u == iRemainingRequestedMs ?
					1u :
					(iRemainingBudgetMs * RequestedCutsMs[iClip]) /
						iRemainingRequestedMs;
				iAllocationMs = (std::clamp)(
					iProportionalMs,
					uint64_t{ 1u },
					iRemainingBudgetMs - iRemainingClipCount);
			}
			OutFittedCutsMs[iClip] = static_cast<uint32_t>(iAllocationMs);
			iRemainingBudgetMs -= iAllocationMs;
			iRemainingRequestedMs -= (std::min)(
				iRemainingRequestedMs,
				static_cast<uint64_t>(RequestedCutsMs[iClip]));
		}
		Status = "Fitted only the loop clip window of the extracted HOLD chain from " +
			std::to_string(iRequestedTotalMs) + " ms to the existing " +
			std::to_string(iStageDurationMs) +
			" ms Server Stage clock; start/end/tail cuts were preserved.";
		return true;
	}

	std::string StableEffectIdFragment(std::string value)
	{
		std::transform(
			value.begin(), value.end(), value.begin(),
			[](const unsigned char ch)
			{
				return static_cast<char>(std::tolower(ch));
			});
		for (char& ch : value)
		{
			if (0 == std::isalnum(static_cast<unsigned char>(ch)) &&
				'_' != ch && '-' != ch)
			{
				ch = '-';
			}
		}
		return value;
	}

	bool_t NormalizeAnimationSlotDraftClock(
		Client::CBalanceTool::PATTERN_STAGE_EDIT& Draft,
		const bool_t bGrowStageToFit,
		std::string& Status)
	{
		if (Draft.animationSlots.empty())
		{
			Draft.animationEndPolicy = "NONE";
			Draft.animationRepeatCount = 0u;
			return true;
		}
		uint32_t iAnimationWallMs = 0u;
		if (!ComputeExactAnimationWallMs(Draft, iAnimationWallMs))
		{
			Status =
				"Animation transfer requires finite, non-loop occurrence clocks on both Stages.";
			return false;
		}
		if (iAnimationWallMs > Draft.durationMs)
		{
			if (!bGrowStageToFit)
			{
				Status =
					"Animation transfer would exceed the preserved source Stage clock.";
				return false;
			}
			Draft.durationMs = iAnimationWallMs;
		}
		Draft.animationEndPolicy = iAnimationWallMs == Draft.durationMs ?
			"EXACT" : "HOLD_LAST_POSE";
		const bool_t bOneRepeatedClip = Draft.animationSlots.size() > 1u &&
			std::all_of(
				Draft.animationSlots.begin() + 1u, Draft.animationSlots.end(),
				[&Draft](const Client::CBalanceTool::ANIMATION_SLOT_EDIT& Slot)
				{
					return Slot.clip == Draft.animationSlots.front().clip;
				});
		Draft.animationRepeatCount = bOneRepeatedClip ?
			static_cast<uint32_t>(Draft.animationSlots.size()) : 1u;
		return true;
	}

	bool_t ResolveValtanColliderScheduleEndpoints(
		const Client::CBalanceTool::PATTERN_STAGE_EDIT& Draft,
		uint32_t& iOutFirstMs,
		uint32_t& iOutLastMs,
		std::string& Status)
	{
		if ("NONE" == Draft.hitShape || 0u == Draft.durationMs)
		{
			Status =
				"Collider timing edit requires one admitted Server hit schedule or active window.";
			return false;
		}
		if (Draft.hasHitActivation)
		{
			const uint64_t iActivationEndMs =
				static_cast<uint64_t>(Draft.hitActivationStartMs) +
				Draft.hitActivationLifetimeMs;
			if (0u == Draft.hitActivationLifetimeMs ||
				Draft.hitActivationStartMs >= Draft.durationMs ||
				iActivationEndMs > Draft.durationMs)
			{
				Status =
					"Collider active window extends past its Server Stage clock.";
				return false;
			}
			iOutFirstMs = Draft.hitActivationStartMs;
			iOutLastMs = static_cast<uint32_t>(iActivationEndMs);
			return true;
		}
		if (0u == Draft.hitCount)
		{
			Status =
				"Collider timing edit requires one admitted Server hit schedule.";
			return false;
		}
		if (!Draft.hitOffsetsMs.empty())
		{
			if (Draft.hitOffsetsMs.size() != Draft.hitCount ||
				Draft.hitOffsetsMs.end() != std::adjacent_find(
					Draft.hitOffsetsMs.begin(), Draft.hitOffsetsMs.end(),
					[](const uint32_t iLeft, const uint32_t iRight)
					{ return iLeft >= iRight; }))
			{
				Status =
					"Collider timing edit rejected an invalid explicit pulse order.";
				return false;
			}
			iOutFirstMs = Draft.hitOffsetsMs.front();
			iOutLastMs = Draft.hitOffsetsMs.back();
		}
		else
		{
			const uint64_t iLastMs =
				static_cast<uint64_t>(Draft.hitDelayMs) +
				static_cast<uint64_t>(Draft.hitCount - 1u) *
					Draft.hitIntervalMs;
			if (iLastMs > (std::numeric_limits<uint32_t>::max)())
			{
				Status = "Collider interval schedule overflowed its Stage clock.";
				return false;
			}
			iOutFirstMs = Draft.hitDelayMs;
			iOutLastMs = static_cast<uint32_t>(iLastMs);
		}
		if (iOutLastMs >= Draft.durationMs)
		{
			Status = "Collider pulse schedule extends past its Server Stage clock.";
			return false;
		}
		return true;
	}

	bool_t TranslateValtanColliderSchedule(
		Client::CBalanceTool::PATTERN_STAGE_EDIT& Draft,
		const int64_t iRequestedDeltaMs,
		std::string& Status)
	{
		uint32_t iFirstMs = 0u;
		uint32_t iLastMs = 0u;
		if (!ResolveValtanColliderScheduleEndpoints(
				Draft, iFirstMs, iLastMs, Status))
		{
			return false;
		}
		const int64_t iLatestDeltaMs = Draft.hasHitActivation ?
			static_cast<int64_t>(Draft.durationMs) - iLastMs :
			static_cast<int64_t>(Draft.durationMs - 1u - iLastMs);
		const int64_t iAppliedDeltaMs = (std::clamp)(
			iRequestedDeltaMs,
			-static_cast<int64_t>(iFirstMs),
			iLatestDeltaMs);
		if (0 == iAppliedDeltaMs)
		{
			Status = Draft.hasHitActivation ?
				"Collider active-window start is unchanged." :
				"Collider pulse schedule start is unchanged.";
			return true;
		}
		if (Draft.hasHitActivation)
		{
			Draft.hitActivationStartMs = static_cast<uint32_t>(
				static_cast<int64_t>(Draft.hitActivationStartMs) +
				iAppliedDeltaMs);
			Status =
				"Moved the Server collider active window by " +
				std::to_string(iAppliedDeltaMs) +
				" ms while preserving its lifetime. Use Save when ready.";
			return true;
		}
		if (Draft.hitOffsetsMs.empty())
		{
			Draft.hitDelayMs = static_cast<uint32_t>(
				static_cast<int64_t>(Draft.hitDelayMs) + iAppliedDeltaMs);
		}
		else
		{
			for (uint32_t& iOffsetMs : Draft.hitOffsetsMs)
			{
				iOffsetMs = static_cast<uint32_t>(
					static_cast<int64_t>(iOffsetMs) + iAppliedDeltaMs);
			}
		}
		Status =
			"Moved every Server collider pulse by " +
			std::to_string(iAppliedDeltaMs) +
			" ms while preserving the exact spacing. Use Save when ready.";
		return true;
	}

	bool_t ResizeValtanColliderScheduleEndpoint(
		Client::CBalanceTool::PATTERN_STAGE_EDIT& Draft,
		const bool_t bResizeStart,
		const uint32_t iRequestedEndpointMs,
		std::string& Status)
	{
		uint32_t iFirstMs = 0u;
		uint32_t iLastMs = 0u;
		if (!ResolveValtanColliderScheduleEndpoints(
				Draft, iFirstMs, iLastMs, Status))
		{
			return false;
		}
		if (Draft.hasHitActivation)
		{
			const uint32_t iClampedEndpointMs = bResizeStart ?
				(std::clamp)(iRequestedEndpointMs, 0u, iLastMs - 1u) :
				(std::clamp)(iRequestedEndpointMs,
					iFirstMs + 1u, Draft.durationMs);
			if ((bResizeStart && iFirstMs == iClampedEndpointMs) ||
				(!bResizeStart && iLastMs == iClampedEndpointMs))
			{
				Status = bResizeStart ?
					"Collider active-window start is unchanged." :
					"Collider active-window end is unchanged.";
				return true;
			}
			if (bResizeStart)
			{
				Draft.hitActivationStartMs = iClampedEndpointMs;
				Draft.hitActivationLifetimeMs = iLastMs - iClampedEndpointMs;
			}
			else
			{
				Draft.hitActivationLifetimeMs =
					iClampedEndpointMs - iFirstMs;
			}
			Status = bResizeStart ?
				"Resized the Server collider active-window start while preserving its end. Use Save when ready." :
				"Resized the Server collider active-window end while preserving its start. Use Save when ready.";
			return true;
		}
		if (Draft.hitCount < 2u)
		{
			Status =
				"A one-pulse Server collider is instantaneous. Drag its body to move the pulse; add a second pulse before resizing start/end.";
			return false;
		}

		std::vector<uint32_t> Offsets;
		Offsets.reserve(Draft.hitCount);
		if (!Draft.hitOffsetsMs.empty())
		{
			Offsets = Draft.hitOffsetsMs;
		}
		else
		{
			for (uint32_t iHit = 0u; iHit < Draft.hitCount; ++iHit)
			{
				Offsets.push_back(Draft.hitDelayMs +
					iHit * Draft.hitIntervalMs);
			}
		}

		const uint32_t iClampedEndpointMs = bResizeStart ?
			(std::clamp)(iRequestedEndpointMs, 0u, Offsets[1u] - 1u) :
			(std::clamp)(iRequestedEndpointMs,
				Offsets[Offsets.size() - 2u] + 1u, Draft.durationMs - 1u);
		const std::size_t iEndpointIndex = bResizeStart ?
			0u : Offsets.size() - 1u;
		if (Offsets[iEndpointIndex] == iClampedEndpointMs)
		{
			Status = bResizeStart ?
				"Collider schedule start is unchanged." :
				"Collider schedule end is unchanged.";
			return true;
		}
		Offsets[iEndpointIndex] = iClampedEndpointMs;
		Draft.hitOffsetsMs = std::move(Offsets);
		Draft.hitCount = static_cast<uint32_t>(Draft.hitOffsetsMs.size());
		Draft.hitDelayMs = 0u;
		Draft.hitIntervalMs = 0u;
		Status = bResizeStart ?
			"Resized the Server collider schedule start without moving its later pulses. Use Save when ready." :
			"Resized the Server collider schedule end without moving its earlier pulses. Use Save when ready.";
		return true;
	}

	void CopyValtanColliderFields(const Client::CBalanceTool::PATTERN_STAGE_EDIT& source,
		Client::CBalanceTool::PATTERN_STAGE_EDIT& target)
	{
		target.hitShape = source.hitShape;
		target.hitOuterRadius = source.hitOuterRadius;
		target.hitInnerRadius = source.hitInnerRadius;
		target.hitAngleDegrees = source.hitAngleDegrees;
		target.hitLength = source.hitLength;
		target.hitHalfWidth = source.hitHalfWidth;
		target.hitCount = source.hitCount;
		target.hitIntervalMs = source.hitIntervalMs;
		target.hitDelayMs = source.hitDelayMs;
		target.hitOffsetsMs = source.hitOffsetsMs;
		target.hasHitAnchor = source.hasHitAnchor;
		target.hitAnchorKind = source.hitAnchorKind;
		target.hitAnchorForwardOffsetM = source.hitAnchorForwardOffsetM;
		target.hitAnchorRightOffsetM = source.hitAnchorRightOffsetM;
		target.hitAnchorYawOffsetDegrees = source.hitAnchorYawOffsetDegrees;
		target.hasHitActivation = source.hasHitActivation;
		target.hitActivationStartMs = source.hitActivationStartMs;
		target.hitActivationLifetimeMs = source.hitActivationLifetimeMs;
		target.damageProfileId = source.damageProfileId;
		target.pushRangeM = source.pushRangeM;
		target.pushMs = source.pushMs;
		target.knockdown = source.knockdown;
		target.downMs = source.downMs;
		target.playerResponse = source.playerResponse;
		target.attachmentSlot = source.attachmentSlot;
		target.hasGripLocalOffset = source.hasGripLocalOffset;
		target.gripForwardM = source.gripForwardM;
		target.gripUpM = source.gripUpM;
		target.gripRightM = source.gripRightM;
	}

	bool BuildValtanColliderPasteDraft(const Client::CBalanceTool::PATTERN_STAGE_EDIT& source,
		const uint32_t atMs, Client::CBalanceTool::PATTERN_STAGE_EDIT& target, std::string& status)
	{
		uint32_t first = 0u, last = 0u;
		if (!source.attackContacts.empty() || source.playerResponse != "DAMAGE" ||
			!ResolveValtanColliderScheduleEndpoints(source, first, last, status))
		{
			status = "Copy requires an ordinary Damage collider; per-contact and Grab owners keep their typed editors. " + status;
			return false;
		}
		const bool hasTarget = target.hitShape != "NONE";
		if (!target.attackContacts.empty() || target.portalRushMotionEditable ||
			(hasTarget ? !target.colliderTuneAdmitted : !target.colliderAddAdmitted))
		{
			status = "The destination Stage does not admit Collider Add/Tune; its current hit contract was preserved.";
			return false;
		}
		if (atMs >= target.durationMs || uint64_t(atMs) + last - first >=
			uint64_t(target.durationMs) + (source.hasHitActivation ? 1u : 0u))
		{
			status = "The copied Collider schedule must fit inside the destination Stage. Move the playhead or extend the Stage first.";
			return false;
		}
		auto candidate = target;
		if (!hasTarget)
		{
			CopyValtanColliderFields(source, candidate);
		}
		else
		{
			if (source.hasHitActivation || target.hasHitActivation)
			{
				status = "A Stage owns one continuous Collider window. Paste it into an empty Stage; existing windows are preserved.";
				return false;
			}
			auto comparable = source;
			comparable.hitCount = target.hitCount;
			comparable.hitIntervalMs = target.hitIntervalMs;
			comparable.hitDelayMs = target.hitDelayMs;
			comparable.hitOffsetsMs = target.hitOffsetsMs;
			if (!SameValtanColliderDraft(comparable, target))
			{
				status = "This Stage already owns a different Collider shape, anchor, damage or reaction. Paste into an empty Stage; no existing Collider was replaced.";
				return false;
			}
		}
		if (source.hasHitActivation)
		{
			candidate.hitActivationStartMs = atMs;
		}
		else
		{
			std::vector<uint32_t> offsets;
			const auto appendOffsets = [&offsets](const Client::CBalanceTool::PATTERN_STAGE_EDIT& value,
				const int64_t shift) {
				for (uint32_t i = 0u; i < value.hitCount; ++i)
				{
					const uint32_t time = value.hitOffsetsMs.empty() ? value.hitDelayMs + i * value.hitIntervalMs : value.hitOffsetsMs[i];
					offsets.push_back(static_cast<uint32_t>(int64_t(time) + shift));
				}
			};
			if (hasTarget) appendOffsets(target, 0);
			appendOffsets(source, int64_t(atMs) - first);
			std::sort(offsets.begin(), offsets.end());
			if (offsets.size() > 64u || std::adjacent_find(offsets.begin(), offsets.end()) != offsets.end())
			{
				status = "Collider Paste overlaps an existing pulse or exceeds 64 pulses. Choose another playhead time; every existing pulse was preserved.";
				return false;
			}
			candidate.hitOffsetsMs = std::move(offsets);
			candidate.hitCount = static_cast<uint32_t>(candidate.hitOffsetsMs.size());
			candidate.hitDelayMs = 0u;
			candidate.hitIntervalMs = 0u;
		}
		target = std::move(candidate);
		return true;
	}

	Client::VALTAN_STAGE_VIEW BuildPatternSoundCandidateStage(
		const Client::VALTAN_STAGE_VIEW& BaselineStage,
		const Client::CBalanceTool::PATTERN_STAGE_EDIT& Draft)
	{
		Client::VALTAN_STAGE_VIEW Candidate = BaselineStage;
		Candidate.strActionId = Draft.actionId;
		Candidate.iDurationMs = Draft.durationMs;
		Candidate.iAuthoringRepeatCount = Draft.animationRepeatCount;
		Candidate.strAnimationEndPolicy = Draft.animationEndPolicy;
		Candidate.bSuppressAnimation = Draft.animationSlots.empty();
		Candidate.ClipOccurrences.clear();
		Candidate.ClipOccurrences.reserve(Draft.animationSlots.size());
		for (const Client::CBalanceTool::ANIMATION_SLOT_EDIT& Slot :
			Draft.animationSlots)
		{
			Client::VALTAN_CLIP_OCCURRENCE_VIEW Occurrence;
			Occurrence.strClipOccurrenceId = Slot.clipOccurrenceId;
			Occurrence.strClipName = Slot.clip;
			Occurrence.strMappingBasis = Slot.mappingBasis;
			Occurrence.iSourceStartMs = Slot.sourceStartMs;
			Occurrence.iPlayMs = Slot.playMs;
			Occurrence.fPlayRate = static_cast<f32_t>(Slot.playRate);
			Occurrence.bLoop = Slot.repeatUntilStageEnd;
			Candidate.ClipOccurrences.push_back(std::move(Occurrence));
		}
		return Candidate;
	}

	bool_t ValidateClipSourceDependencyWindow(
		const Client::VALTAN_STAGE_VIEW& CandidateStage,
		const std::string& strClipOccurrenceId,
		const uint32_t iSourceStartMs,
		const bool_t bHasSourceEnd,
		const uint32_t iSourceEndMs,
		const bool_t bRequireLoop,
		const std::string& strDependencyLabel,
		std::string& Status, const bool_t bAllowEffectTail = false)
	{
		const std::size_t iOccurrenceCount = static_cast<std::size_t>(
			std::count_if(
				CandidateStage.ClipOccurrences.begin(),
				CandidateStage.ClipOccurrences.end(),
				[&strClipOccurrenceId](
					const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
				{
					return Clip.strClipOccurrenceId == strClipOccurrenceId;
				}));
		const auto Found = std::find_if(
			CandidateStage.ClipOccurrences.begin(),
			CandidateStage.ClipOccurrences.end(),
			[&strClipOccurrenceId](
				const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
			{
				return Clip.strClipOccurrenceId == strClipOccurrenceId;
			});
		if (1u != iOccurrenceCount ||
			CandidateStage.ClipOccurrences.end() == Found)
		{
			Status = strDependencyLabel +
				" no longer resolves exactly one clipOccurrenceId: " +
				strClipOccurrenceId + ".";
			return false;
		}

		const uint64_t iWindowBeginMs = Found->iSourceStartMs;
		const bool_t bFiniteWindow = 0u != Found->iPlayMs;
		const uint64_t iWindowEndExclusiveMs = iWindowBeginMs +
			static_cast<uint64_t>(Found->iPlayMs);
		const bool_t bStartInside =
			static_cast<uint64_t>(iSourceStartMs) >= iWindowBeginMs &&
			(!bFiniteWindow ||
			 static_cast<uint64_t>(iSourceStartMs) < iWindowEndExclusiveMs);
		const bool_t bEndInside = !bHasSourceEnd ||
			(iSourceEndMs > iSourceStartMs && iSourceEndMs <= 600000u &&
			 (bAllowEffectTail || !bFiniteWindow ||
			  static_cast<uint64_t>(iSourceEndMs) <= iWindowEndExclusiveMs));
		if (!bStartInside || !bEndInside ||
			(bRequireLoop && !Found->bLoop))
		{
			Status = strDependencyLabel +
				" is outside the candidate Animation source window or requires a removed loop: " +
				strClipOccurrenceId + ".";
			return false;
		}
		return true;
	}

	uint32_t ResolveEffectPlaybackOffsetMs(const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& cue)
	{
		if (cue.bHasPlaybackOffset) return cue.iPlaybackOffsetMs;
		return !cue.bUsesStageClock && cue.strEffectAssetId.starts_with("effect.valtan.action.") &&
			cue.strEffectAssetId.ends_with(".full.restore") &&
			!cue.strBindingId.starts_with("cue.valtan.composition.") ? cue.iSourceStartMs : 0u;
	}

	bool_t MoveValtanEffectInvocation(
		Client::CAnimation_Tool* animation, Client::CBalanceTool* balance,
		const Client::VALTAN_PATTERN_VIEW& pattern, const Client::VALTAN_STAGE_VIEW& sourceStage,
		const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& current,
		const std::vector<Client::CValtanActionWorkbench::TIMELINE_ITEM>& items,
		const uint32_t patternStartMs, std::string& targetStageId,
		std::string& targetOccurrenceId, std::string& status,
		const std::optional<uint32_t> fixedPatternEndMs = std::nullopt)
	{
		using Workbench = Client::CValtanActionWorkbench;
		const Workbench::TIMELINE_ITEM* target = nullptr;
		for (const auto& item : items)
			if (item.eLane == (current.bUsesStageClock ? Workbench::TIMELINE_LANE::STAGE : Workbench::TIMELINE_LANE::ANIMATION) &&
				item.strPatternId == pattern.strPatternId && patternStartMs >= item.iStartMs &&
				patternStartMs < item.iEndMs && (!target || item.iStartMs >= target->iStartMs)) target = &item;
		if (!target || !balance)
		{ status = current.bUsesStageClock ? "Effect drag must begin inside a Stage; the existing cue was preserved." :
			"Linked Effect drag must begin inside an Animation box; the existing cue was preserved."; return false; }
		const auto stage = std::find_if(pattern.Stages.begin(), pattern.Stages.end(),
			[&](const auto& row) { return row.strStageId == target->strStageId; });
		if (stage == pattern.Stages.end()) return false;
		const auto clip = std::find_if(stage->ClipOccurrences.begin(), stage->ClipOccurrences.end(),
			[&](const auto& row) { return row.strClipOccurrenceId == target->strStableId; });
		if (!current.bUsesStageClock && clip == stage->ClipOccurrences.end()) return false;
		const double rate = current.bUsesStageClock ? 1.0 : double(clip->fPlayRate);
		auto candidate = current;
		candidate.iPlaybackOffsetMs = ResolveEffectPlaybackOffsetMs(current);
		candidate.bHasPlaybackOffset = true;
		candidate.strStageId = stage->strStageId;
		candidate.strActionId = stage->strActionId;
		candidate.strClipOccurrenceId = current.bUsesStageClock ? std::string{} : clip->strClipOccurrenceId;
		candidate.iStageDurationMs = stage->iDurationMs;
		const uint64_t sourceMs = (current.bUsesStageClock ? 0u : clip->iSourceStartMs) + static_cast<uint64_t>(std::llround(
			(patternStartMs - target->iStartMs) * rate));
		if (sourceMs > 600000u) { status = "Effect drag exceeds its source timing bound."; return false; }
		candidate.iSourceStartMs = static_cast<uint32_t>(sourceMs);
		if (candidate.bUsesStageClock) candidate.iStageOffsetMs = candidate.iSourceStartMs;
		candidate.iSourceEndMs = current.bHasSourceEnd ? candidate.iSourceStartMs +
			(current.iSourceEndMs - current.iSourceStartMs) : 0u;
		if (fixedPatternEndMs)
		{
			if (*fixedPatternEndMs <= patternStartMs) return false;
			candidate.bHasSourceEnd = true;
			candidate.strStopPolicy = "cue_end";
			candidate.eStopPolicy = EFFECT_STOP_POLICY::CUE_END;
			const uint64_t end = sourceMs + static_cast<uint64_t>(std::llround(
				(*fixedPatternEndMs - patternStartMs) * rate));
			if (end > 600000u) { status = "Effect end exceeds 600000 ms; the cue was preserved."; return false; }
			candidate.iSourceEndMs = static_cast<uint32_t>(end);
		}
		if (!ValidateValtanEffectInvocationClock(animation, *stage, candidate, status)) return false;
		bool_t applied = false;
		if (stage->strStageId == sourceStage.strStageId)
			applied = balance->Update_ValtanStageEffectCue(pattern.strPatternId, stage->strStageId,
				stage->strActionId, current.strBindingId, current.strOccurrenceId, candidate, status);
		else
		{
			if (!current.strBindingId.starts_with("cue.valtan.composition."))
			{ status = "Moving a source-owned cue across Stages requires an independent composition invocation."; return false; }
			candidate.strBindingId = BuildNextCompositionEffectCueId(pattern, *stage);
			if (candidate.strBindingId.empty()) { status = "No free destination Effect invocation identity."; return false; }
			candidate.strOccurrenceId = candidate.strBindingId + ".occurrence.01";
			applied = balance->Apply_ValtanCompositionDraftTransaction([&](std::string& message) {
				return balance->Remove_ValtanStageEffectCue(pattern.strPatternId, sourceStage.strStageId,
					sourceStage.strActionId, current.strBindingId, current.strOccurrenceId,
					current.strEffectAssetId, current.strClipOccurrenceId, message) &&
					balance->Add_ValtanStageEffectCue(pattern.strPatternId, stage->strStageId,
						stage->strActionId, candidate, message);
			}, status);
		}
		if (applied)
		{
			targetStageId = stage->strStageId;
			targetOccurrenceId = candidate.strOccurrenceId;
			status = "Moved Effect to " + targetStageId + " at Pattern " + std::to_string(patternStartMs) +
				" ms. The current draft is ready for Preview; Save commits it.";
		}
		return applied;
	}

	Client::VALTAN_PATTERN_PREVIEW_PATH InitialPreviewPath(
		const Client::VALTAN_PATTERN_VIEW& Pattern)
	{
		std::vector<const Client::VALTAN_STAGE_VIEW*> Normal, Capture;
		std::string Status;
		if (Client::CValtanPatternTree::Build_PreviewStagePath(
				Pattern, Client::VALTAN_PATTERN_PREVIEW_PATH::NORMAL, Normal, Status) &&
			Client::CValtanPatternTree::Build_PreviewStagePath(
				Pattern, Client::VALTAN_PATTERN_PREVIEW_PATH::CAPTURE_SUCCESS, Capture, Status) &&
			Capture.size() > Normal.size())
			return Client::VALTAN_PATTERN_PREVIEW_PATH::CAPTURE_SUCCESS;
		return Client::VALTAN_PATTERN_PREVIEW_PATH::NORMAL;
	}
}

// 분리된 CPP가 함께 사용하는 함수는 이름 있는 구현 전용 이름 공간에 둔다.
// 익명 이름 공간에 그대로 두면 다른 CPP에서 링크할 수 없으므로 private EditSupport 선언과 이 단일 정의를 연결한다.
// 상태를 가진 새 manager를 만드는 분리가 아니며 기본 인자는 선언 쪽에만 둔다.
namespace Client::ValtanWorkbenchDetail
{

	bool IsValtanServerPlaybackBusy(const Client::CValtanBossTool* boss)
	{
		return (boss && boss->Is_PlayPreparationPending()) ||
			Client::CValtanPatternAuditionService::Get().Has_PlaybackOwnership() ||
			Client::CValtanPatternFlowService::Get().Has_PlaybackOwnership();
	}

	bool ContainsInsensitive(
		const std::string_view text,
		const std::string_view query)
	{
		if (query.empty())
			return true;
		return std::string::npos != Lower(std::string(text)).find(
			Lower(std::string(query)));
	}

	std::string BuildBranchTargetLabel(
		const Client::VALTAN_STAGE_BRANCH_VIEW& Branch)
	{
		if (Branch.strNextPatternId.has_value())
			return "PATTERN " + *Branch.strNextPatternId;
		if (Branch.strNextActionId.has_value())
			return *Branch.strNextActionId;
		return "PATTERN END";
	}

	std::string BuildBranchStableId(
		const std::string& stageId, const std::string& outcome)
	{
		return stageId + "/branch/" + outcome;
	}

	std::string BuildNextCompositionSlotId(
		const std::string& patternId,
		const std::string& stageId,
		const std::vector<Client::CBalanceTool::ANIMATION_SLOT_EDIT>& Slots)
	{
		/* slot은 다음 Sequence를 추가하기 전에 삭제되거나 순서가 바뀔 수 있다. vector 크기는 ID 발급기가 아니다.
		   현재 개수+1개의 후보를 검사해 실제로 사용하지 않는 ID를 고른다. */
		for (std::size_t ordinal = 0u; ordinal <= Slots.size(); ++ordinal)
		{
			const std::string candidate = BuildCompositionSlotId(
				patternId, stageId, ordinal);
			const bool_t exists = std::any_of(
				Slots.begin(), Slots.end(),
				[&candidate](const Client::CBalanceTool::ANIMATION_SLOT_EDIT& Slot)
				{ return Slot.clipOccurrenceId == candidate; });
			if (!exists)
				return candidate;
		}
		return {};
	}

	std::string BuildEffectV2BindingStableId(
		const Client::EFFECT_V2_BINDING& Binding,
		const std::string_view strClipOccurrenceId)
	{
		if (Binding.strBindingId.empty())
			return {};
		std::string StableId = "effect-v2/" + Binding.strBindingId;
		if (!strClipOccurrenceId.empty())
			StableId += "/occurrence/" + std::string(strClipOccurrenceId);
		return StableId;
	}

	const Client::EFFECT_V2_BINDING* ResolveEffectV2Binding(
		const Client::EFFECT_V2_CATALOG_SNAPSHOT& Snapshot,
		const std::string_view strStableId,
		const std::string_view strClipOccurrenceId)
	{
		const Client::EFFECT_V2_BINDING* pResolved = nullptr;
		for (const Client::EFFECT_V2_BINDING& Binding :
			Snapshot.Get_BossValtanBindings())
		{
			if ((Client::EFFECT_V2_CLOCK_BASIS::CLIP_OCCURRENCE == Binding.eClockBasis &&
				 Binding.strClipOccurrenceId != strClipOccurrenceId) ||
				(Client::EFFECT_V2_CLOCK_BASIS::STAGE == Binding.eClockBasis &&
				 !strClipOccurrenceId.empty()))
				continue;
			if (BuildEffectV2BindingStableId(
					Binding, strClipOccurrenceId) != strStableId)
			{
				continue;
			}
			if (nullptr != pResolved)
				return nullptr;
			pResolved = &Binding;
		}
		return pResolved;
	}

	std::string StageSequenceRoleLabel(
		const Client::VALTAN_STAGE_VIEW& Stage)
	{
		const std::string& Role = Stage.strSequenceRole.empty() ?
			Stage.strStageKind : Stage.strSequenceRole;
		return "WAIT" == Role ? "WAIT (blank gap)" : Role;
	}

	std::string BuildNextCompositionEffectCueId(
		const Client::VALTAN_PATTERN_VIEW& pattern,
		const Client::VALTAN_STAGE_VIEW& stage)
	{
		constexpr std::string_view OCCURRENCE_SUFFIX = ".occurrence.01";
		const std::string prefix = "cue.valtan.composition." +
			StableEffectIdFragment(pattern.strPatternId) + "." +
			StableEffectIdFragment(stage.strStageId) + ".";
		const auto IsUsed = [&stage](const std::string& candidate)
		{
			return std::any_of(
				stage.ProductCues.begin(), stage.ProductCues.end(),
				[&candidate](const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& cue)
				{
					return cue.strBindingId == candidate;
				});
		};
		const auto FindFreeId = [&IsUsed, OCCURRENCE_SUFFIX](
			const std::string& candidatePrefix) -> std::string
		{
			for (std::uint32_t ordinal = 1u; ordinal <= 9999u; ++ordinal)
			{
				std::ostringstream id;
				id << candidatePrefix << std::setw(2) << std::setfill('0') <<
					ordinal;
				const std::string candidate = id.str();
				if (candidate.size() + OCCURRENCE_SUFFIX.size() > 160u)
					return {};
				if (!IsUsed(candidate))
					return candidate;
			}
			return {};
		};
		if (std::string readable = FindFreeId(prefix); !readable.empty())
			return readable;
		uint64_t hash = 1469598103934665603ull;
		for (const unsigned char ch :
			pattern.strPatternId + "/" + stage.strStageId)
		{
			hash ^= ch;
			hash *= 1099511628211ull;
		}
		std::ostringstream compact;
		compact << "cue.valtan.composition." << std::hex << std::setw(16) <<
			std::setfill('0') << hash << std::dec << ".";
		return FindFreeId(compact.str());
	}

	bool_t ComputeExactAnimationWallMs(
		const Client::CBalanceTool::PATTERN_STAGE_EDIT& Draft,
		uint32_t& iOutWallMs)
	{
		uint64_t iWallMs = 0u;
		if (!Draft.animationEditable || Draft.animationSlots.empty())
			return false;
		for (const Client::CBalanceTool::ANIMATION_SLOT_EDIT& Slot :
			Draft.animationSlots)
		{
			if (0u == Slot.playMs || Slot.repeatUntilStageEnd ||
				!std::isfinite(Slot.playRate) || Slot.playRate <= 0.0)
			{
				return false;
			}
			iWallMs += static_cast<uint64_t>(std::llround(
				static_cast<double>(Slot.playMs) / Slot.playRate));
			if (iWallMs > 600000u)
				return false;
		}
		iOutWallMs = static_cast<uint32_t>(iWallMs);
		return 0u != iOutWallMs;
	}

	bool_t ApplyStageClockPolicy(
		Client::CBalanceTool::PATTERN_STAGE_EDIT& Draft,
		const uint32_t iRequestedDurationMs,
		std::string& Status)
	{
		if (!Draft.animationEditable || Draft.animationSlots.empty() ||
			"NONE" == Draft.animationEndPolicy ||
			"LOOP_TO_STAGE_END" == Draft.animationEndPolicy)
		{
			Draft.durationMs = iRequestedDurationMs;
			return true;
		}
		uint32_t iAnimationWallMs = 0u;
		if (!ComputeExactAnimationWallMs(Draft, iAnimationWallMs))
		{
			Status =
				"Stage clock edit blocked: the Animation Sequence wall is unresolved. Fix zero/native/loop slots first.";
			return false;
		}
		if (iRequestedDurationMs < iAnimationWallMs)
		{
			Status = "Stage clock cannot trim below its Animation Sequence wall (" +
				std::to_string(iAnimationWallMs) + " ms).";
			return false;
		}
		Draft.durationMs = iRequestedDurationMs;
		Draft.animationEndPolicy = iRequestedDurationMs == iAnimationWallMs ?
			"EXACT" : "HOLD_LAST_POSE";
		return true;
	}

	bool_t InitializeDefaultValtanColliderBoxDraft(
		Client::CBalanceTool::PATTERN_STAGE_EDIT& Draft,
		const std::vector<std::string>& DamageProfiles,
		const int32_t iRequestedDamageProfile,
		std::string& Status)
	{
		if (!Draft.colliderAddAdmitted || "NONE" != Draft.hitShape)
		{
			Status =
				"New Server collider BOX creation is not admitted for this Stage.";
			return false;
		}
		if (DamageProfiles.empty())
		{
			Status =
				"New Server collider BOX creation requires one admitted damage.valtan. profile.";
			return false;
		}

		const int32_t iDamageProfile = (std::clamp)(
			iRequestedDamageProfile, 0,
			static_cast<int32_t>(DamageProfiles.size() - 1u));
		Draft.hitShape = "BOX";
		Draft.hitOuterRadius = 0.0;
		Draft.hitInnerRadius = 0.0;
		Draft.hitAngleDegrees = 0.0;
		Draft.hitLength = 8.0;
		Draft.hitHalfWidth = 2.5;
		Draft.hitCount = 1u;
		Draft.hitIntervalMs = 0u;
		Draft.hitDelayMs = 0u;
		Draft.hitOffsetsMs.clear();
		Draft.hasHitActivation = false;
		Draft.hitActivationStartMs = 0u;
		Draft.hitActivationLifetimeMs = 0u;
		Draft.hasHitAnchor = false;
		Draft.hitAnchorKind = "BOSS_CURRENT";
		Draft.hitAnchorForwardOffsetM = 0.0;
		Draft.hitAnchorRightOffsetM = 0.0;
		Draft.hitAnchorYawOffsetDegrees = 0.0;
		Draft.damageProfileId = DamageProfiles[
			static_cast<std::size_t>(iDamageProfile)];
		Draft.pushRangeM = 0.0;
		Draft.pushMs = 0u;
		Draft.knockdown = false;
		Draft.downMs = 0u;
		Draft.playerResponse = "DAMAGE";
		Draft.attachmentSlot = "NONE";
		Status =
			"Created a Server collider BOX draft (8.00 m x 2.50 m half-width).";
		return true;
	}

	bool_t SameValtanColliderDraft(
		const Client::CBalanceTool::PATTERN_STAGE_EDIT& Left,
		const Client::CBalanceTool::PATTERN_STAGE_EDIT& Right)
	{
		return Left.hitShape == Right.hitShape &&
			Left.hitOuterRadius == Right.hitOuterRadius &&
			Left.hitInnerRadius == Right.hitInnerRadius &&
			Left.hitAngleDegrees == Right.hitAngleDegrees &&
			Left.hitLength == Right.hitLength &&
			Left.hitHalfWidth == Right.hitHalfWidth &&
			Left.hitCount == Right.hitCount &&
			Left.hitIntervalMs == Right.hitIntervalMs &&
			Left.hitDelayMs == Right.hitDelayMs &&
			Left.hitOffsetsMs == Right.hitOffsetsMs &&
			Left.hasHitAnchor == Right.hasHitAnchor &&
			Left.hitAnchorKind == Right.hitAnchorKind &&
			Left.hitAnchorForwardOffsetM ==
				Right.hitAnchorForwardOffsetM &&
			Left.hitAnchorRightOffsetM == Right.hitAnchorRightOffsetM &&
			Left.hitAnchorYawOffsetDegrees ==
				Right.hitAnchorYawOffsetDegrees &&
			Left.hasHitActivation == Right.hasHitActivation &&
			Left.hitActivationStartMs == Right.hitActivationStartMs &&
			Left.hitActivationLifetimeMs ==
				Right.hitActivationLifetimeMs &&
			Left.damageProfileId == Right.damageProfileId &&
			Left.pushRangeM == Right.pushRangeM &&
			Left.pushMs == Right.pushMs &&
			Left.knockdown == Right.knockdown &&
			Left.downMs == Right.downMs &&
			Left.playerResponse == Right.playerResponse &&
			Left.attachmentSlot == Right.attachmentSlot &&
			Left.hasGripLocalOffset == Right.hasGripLocalOffset &&
			Left.gripForwardM == Right.gripForwardM &&
			Left.gripUpM == Right.gripUpM &&
			Left.gripRightM == Right.gripRightM;
	}

	bool_t ValidateValtanEffectInvocationClock(
		Client::CAnimation_Tool* animation, const Client::VALTAN_STAGE_VIEW& stage,
		const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& cue, std::string& status)
	{
		if (cue.bUsesStageClock)
		{
			if (!cue.strClipOccurrenceId.empty() || cue.iStageOffsetMs >= stage.iDurationMs ||
				cue.iSourceStartMs != cue.iStageOffsetMs || cue.strRepeatPolicy != "once" ||
				(cue.bHasSourceEnd && (cue.iSourceEndMs <= cue.iStageOffsetMs || cue.iSourceEndMs > 600000u)))
			{
				status = "Independent Effect start must be inside its Stage; its explicit end must follow the start and remain within 600000 ms.";
				return false;
			}
			return true;
		}
		uint64_t wallCursor = 0u;
		for (const auto& clip : stage.ClipOccurrences)
		{
			uint32_t nativeMs = 0u;
			if (!animation || !animation->Resolve_ValtanCompositionNativeClipDurationMs(
				clip.strClipName, nativeMs, status))
			{
				status = "Effect timing requires the selected Animation source duration. " + status;
				return false;
			}
			if (!std::isfinite(clip.fPlayRate) || clip.fPlayRate <= 0.f ||
				nativeMs <= clip.iSourceStartMs || wallCursor >= stage.iDurationMs)
			{
				status = "Effect timing has no reachable Animation source window.";
				return false;
			}
			const uint32_t sourceSpan = clip.iPlayMs ?
				(std::min)(clip.iPlayMs, nativeMs - clip.iSourceStartMs) :
				nativeMs - clip.iSourceStartMs;
			const uint64_t wallSpan = clip.bLoop ? stage.iDurationMs - wallCursor :
				static_cast<uint64_t>(std::llround(sourceSpan / double(clip.fPlayRate)));
			if (clip.strClipOccurrenceId == cue.strClipOccurrenceId)
			{
				const bool unwrapped = clip.bLoop && cue.strRepeatPolicy == "once" &&
					cue.strBindingId.starts_with("cue.valtan.composition.");
				const double availableSource = (std::min)(
					double(stage.iDurationMs - wallCursor) * clip.fPlayRate,
					unwrapped ? double(stage.iDurationMs - wallCursor) * clip.fPlayRate : double(sourceSpan));
				const double end = clip.iSourceStartMs + availableSource;
				if (cue.iSourceStartMs < clip.iSourceStartMs || cue.iSourceStartMs >= end ||
					(cue.bHasSourceEnd && (cue.iSourceEndMs <= cue.iSourceStartMs ||
						cue.iSourceEndMs > (cue.strRepeatPolicy == "once" ? 600000.0 : end))) ||
					(cue.strRepeatPolicy == "each_loop" && !clip.bLoop))
				{
					status = "Effect start/end is outside its Animation and Stage window. Move the box onto the Stage that owns the desired time.";
					return false;
				}
				return true;
			}
			wallCursor += wallSpan;
		}
		status = "Effect timing lost its exact Animation occurrence.";
		return false;
	}

	bool_t SetValtanStageDraftWithSoundDependencyAdmission(
		Client::CAnimation_Tool* const pAnimationTool,
		Client::CBalanceTool* const pBalanceTool,
		const Client::VALTAN_PATTERN_SHAKE_CUE_DOCUMENT* const pPatternShakes,
		const Client::VALTAN_PATTERN_VIEW& BaselinePattern,
		const Client::VALTAN_STAGE_VIEW& BaselineStage,
		const Client::CBalanceTool::PATTERN_STAGE_EDIT& Draft,
		std::string& Status,
		const uint32_t iSourceActionId,
		const uint32_t iSourceSequenceIndex,
		const bool_t bValidateOnly)
	{
		if (nullptr == pAnimationTool || nullptr == pBalanceTool)
		{
			Status =
				"Pattern/Animation mutation requires both the typed Stage owner and Pattern Sound dependency owner.";
			return false;
		}
		const Client::VALTAN_STAGE_VIEW CandidateStage =
			BuildPatternSoundCandidateStage(BaselineStage, Draft);
		const auto fits = [&Draft](uint32_t begin, uint32_t duration)
		{ return duration > 0u && static_cast<uint64_t>(begin) + duration <= Draft.durationMs; };
		for (const auto& cue : CandidateStage.SceneProfileOccurrences)
			if (!fits(cue.iStartMs, cue.iDurationMs))
			{ Status = "Stage duration would cut Scene Profile occurrence " + cue.strOccurrenceId + ". Move or trim that box first."; return false; }
		for (const auto& cue : CandidateStage.LightOccurrences)
			if (!fits(cue.iStartMs, cue.iDurationMs))
			{ Status = "Stage duration would cut Light occurrence " + cue.strOccurrenceId + ". Move or trim that box first."; return false; }
		for (const auto& cue : CandidateStage.CameraInvocations)
			if (cue.strDurationPolicy == "EXPLICIT" && !fits(cue.iStartOffsetMs, cue.iDurationMs))
			{ Status = "Stage duration would cut Camera occurrence " + cue.strCameraInvocationId + ". Move or trim that box first."; return false; }
		for (const auto& cue : CandidateStage.CombatObjectEffects)
		{
			const uint64_t lastSpawn = static_cast<uint64_t>(cue.iFirstSpawnOffsetMs) +
				static_cast<uint64_t>(cue.iSpawnScheduleCount > 0u ? cue.iSpawnScheduleCount - 1u : 0u) * cue.iSpawnIntervalMs;
			if (lastSpawn >= Draft.durationMs)
			{ Status = "Stage duration would discard a Summon spawn for " + cue.strCombatObjectArchetypeId + ". Move that box first."; return false; }
		}
		if (!pAnimationTool->
				Validate_ValtanCompositionAnimationStageMutation(
					BaselineStage, CandidateStage, Status))
		{
			Status =
				"Stage mutation rejected before the typed draft changed: " + Status;
			return false;
		}
		if (!pAnimationTool->
				Validate_ValtanCompositionPatternSoundStageDependencies(
					BaselinePattern, BaselineStage, CandidateStage, Status))
		{
			Status =
				"Stage mutation rejected before the typed draft changed: " + Status;
			return false;
		}
		const bool_t bAnimationDependencyChanged =
			BaselineStage.strActionId != CandidateStage.strActionId ||
			BaselineStage.ClipOccurrences.size() !=
				CandidateStage.ClipOccurrences.size() ||
			!std::equal(
				BaselineStage.ClipOccurrences.begin(),
				BaselineStage.ClipOccurrences.end(),
				CandidateStage.ClipOccurrences.begin(),
				[](const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Baseline,
					const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Candidate)
				{
					return Baseline.strClipOccurrenceId ==
							Candidate.strClipOccurrenceId &&
						Baseline.strClipName == Candidate.strClipName &&
						Baseline.strMappingBasis == Candidate.strMappingBasis &&
						Baseline.iSourceStartMs == Candidate.iSourceStartMs &&
						Baseline.iPlayMs == Candidate.iPlayMs &&
						Baseline.fPlayRate == Candidate.fPlayRate &&
						Baseline.bLoop == Candidate.bLoop;
				});
		if (bAnimationDependencyChanged)
		{
			for (const Client::VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue :
				CandidateStage.ProductCues)
			{
				if (Cue.bUsesStageClock)
					continue;
				if (!ValidateClipSourceDependencyWindow(
						CandidateStage, Cue.strClipOccurrenceId,
						Cue.iSourceStartMs, Cue.bHasSourceEnd,
						Cue.iSourceEndMs,
						"each_loop" == Cue.strRepeatPolicy,
						"Effect occurrence " + Cue.strOccurrenceId, Status, Cue.strRepeatPolicy == "once"))
				{
					Status =
						"Stage mutation rejected before the typed draft changed: " +
						Status;
					return false;
				}
			}
		}
		if (bAnimationDependencyChanged && nullptr == pPatternShakes)
		{
			Status =
				"Stage mutation rejected before the typed draft changed: the Pattern Shake dependency source is unavailable.";
			return false;
		}
		if (bAnimationDependencyChanged)
		{
			for (const Client::VALTAN_PATTERN_SHAKE_CUE& Cue :
				pPatternShakes->Cues)
			{
				if (Cue.strPatternId != BaselinePattern.strPatternId ||
					Cue.strStageId != BaselineStage.strStageId)
				{
					continue;
				}
				if (Cue.strActionId != BaselineStage.strActionId ||
					Cue.strActionId != CandidateStage.strActionId)
				{
					Status =
						"Stage mutation rejected before the typed draft changed: Pattern Shake row no longer resolves the exact Pattern/Stage/action tuple: " +
						Cue.strOccurrenceId + ".";
					return false;
				}
				const auto FindOccurrence = [&Cue](
					const Client::VALTAN_STAGE_VIEW& Stage)
				{
					return std::find_if(
						Stage.ClipOccurrences.begin(), Stage.ClipOccurrences.end(),
						[&Cue](const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
						{
							return Clip.strClipOccurrenceId ==
								Cue.strClipOccurrenceId;
						});
				};
				const std::size_t iBaselineCount = static_cast<std::size_t>(
					std::count_if(
						BaselineStage.ClipOccurrences.begin(),
						BaselineStage.ClipOccurrences.end(),
						[&Cue](const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
						{
							return Clip.strClipOccurrenceId ==
								Cue.strClipOccurrenceId;
						}));
				const std::size_t iCandidateCount = static_cast<std::size_t>(
					std::count_if(
						CandidateStage.ClipOccurrences.begin(),
						CandidateStage.ClipOccurrences.end(),
						[&Cue](const Client::VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
						{
							return Clip.strClipOccurrenceId ==
								Cue.strClipOccurrenceId;
						}));
				const auto BaselineClip = FindOccurrence(BaselineStage);
				const auto CandidateClip = FindOccurrence(CandidateStage);
				if (1u != iBaselineCount || 1u != iCandidateCount ||
					BaselineStage.ClipOccurrences.end() == BaselineClip ||
					CandidateStage.ClipOccurrences.end() == CandidateClip)
				{
					Status =
						"Stage mutation rejected before the typed draft changed: Pattern Shake row would dangle; retarget or remove it explicitly before replacing its clip occurrence: " +
						Cue.strOccurrenceId + ".";
					return false;
				}
				if (BaselineClip->strClipName != CandidateClip->strClipName &&
					"PROJECT_AUTHORED" != CandidateClip->strMappingBasis)
				{
					Status =
						"Stage mutation rejected before the typed draft changed: Pattern Shake-qualified clipOccurrenceId can change its resource only through an explicit PROJECT_AUTHORED slot replacement: " +
							Cue.strClipOccurrenceId + ".";
					return false;
				}
				if (!ValidateClipSourceDependencyWindow(
						CandidateStage, Cue.strClipOccurrenceId,
						Cue.iStartMs, false, 0u,
						Client::VALTAN_PATTERN_SHAKE_REPEAT_POLICY::EACH_LOOP ==
							Cue.eRepeatPolicy,
						"Pattern Shake occurrence " + Cue.strOccurrenceId,
						Status))
				{
					Status =
						"Stage mutation rejected before the typed draft changed: " +
						Status;
					return false;
				}
			}
		}
		if (bValidateOnly)
		{
			Status =
				"Validated native Animation, Effect, Sound, and Shake dependencies without mutating the Stage draft.";
			return true;
		}
		if (0u != iSourceActionId)
		{
			return pBalanceTool->Set_ValtanStageSequenceDraft(
				BaselinePattern.strPatternId, BaselineStage.strStageId,
				iSourceActionId, iSourceSequenceIndex, Draft, Status);
		}
		return pBalanceTool->Set_ValtanStageDraft(
			BaselinePattern.strPatternId, BaselineStage.strStageId,
			Draft, Status);
	}

	const char* PreviewPathLabel(
		const Client::VALTAN_PATTERN_PREVIEW_PATH ePath)
	{
		switch (ePath)
		{
		case Client::VALTAN_PATTERN_PREVIEW_PATH::NORMAL:
			return "Normal / Timeout";
		case Client::VALTAN_PATTERN_PREVIEW_PATH::COUNTER_GROGGY:
			return "Counter Hit -> Groggy";
		case Client::VALTAN_PATTERN_PREVIEW_PATH::WALL_GROGGY:
			return "Wall Contact -> Groggy";
		case Client::VALTAN_PATTERN_PREVIEW_PATH::PART_BREAK:
			return "Wall Contact -> Part Break";
		case Client::VALTAN_PATTERN_PREVIEW_PATH::CAPTURE_SUCCESS:
			return "Capture Success -> Throw";
		default:
			return "Invalid";
		}
	}
}



// MainApp::EnsureDebugTool이 이미 생성한 세 소유 도구의 주소를 넘긴다.
// 생성자는 주소만 보관하며 파일 로드나 preview 시작은 하지 않는다. 실제 준비는 선택된 세션의 frame 처리에서 수행한다.
Client::CValtanActionWorkbench::CValtanActionWorkbench(
	CAnimation_Tool* const pAnimationTool,
	CBalanceTool* const pBalanceTool,
	CValtanBossTool* const pValtanBossTool)
	: m_pAnimationTool(pAnimationTool),
	  m_pBalanceTool(pBalanceTool),
	  m_pValtanBossTool(pValtanBossTool)
{
}

void Client::CValtanActionWorkbench::Reload_SemanticValtanEffects(
	const bool_t bReloadCatalog)
{
	m_bSemanticValtanEffectLoadAttempted = true;
	if (bReloadCatalog || !m_bCinematicEffectLibraryLoadAttempted)
	{
		m_bCinematicEffectLibraryLoadAttempted = true;
		if (Load_ValtanCinematicEffectLibrary(m_CinematicEffectLibrary, m_strCinematicEffectLibraryStatus))
			m_strCinematicEffectLibraryStatus.clear();
	}
	const auto previousV1Ids = m_SemanticValtanEffectAssetIds;
	m_SemanticValtanEffectAssetIds.clear();
	m_FilteredEffectAssetIndices.clear();
	m_FilteredEffectV2DocumentIndices.clear();
	m_FilteredEffectV2GroupIndices.clear();
	m_bEffectFilterDirty = true;
	for (const std::string& EffectAssetId : CEffectCatalog::Get_EffectAssetIds())
	{
		if (CEffectCatalog::Is_DirectAuthoredDocument(EffectAssetId))
		{
			m_SemanticValtanEffectAssetIds.push_back(EffectAssetId);
		}
	}
	std::sort(
		m_SemanticValtanEffectAssetIds.begin(),
		m_SemanticValtanEffectAssetIds.end());
	m_SemanticValtanEffectAssetIds.erase(
		std::unique(
			m_SemanticValtanEffectAssetIds.begin(),
			m_SemanticValtanEffectAssetIds.end()),
		m_SemanticValtanEffectAssetIds.end());

	// 목록을 갱신할 때 저장된 작은 header와 category 정보만 읽는다.
	// resource 목록을 매 프레임 그리거나 필터링할 때 전체 Effect 내용을 읽지 않는다.
	VALTAN_FULL_RESTORE_CLIP_INDEX restoreClips;
	std::string restoreStatus;
	const bool restoreReady = CValtanPatternTree::Load_FullRestoreSourceClips(
		CProjectDataRoot::Resolve("Effects/ValtanFullRestoreAnimations.json"), restoreClips, restoreStatus);
	std::vector<CEffectAuthoringResourceTree::RESOURCE> inventory, organization;
	std::string inventoryStatus, organizationStatus;
	const bool inventoryReady = CEffectAuthoringResourceTree::Read_V1Inventory(inventory, inventoryStatus);
	const bool organizationReady = CEffectAuthoringResourceTree::Read_V1Organization(organization, organizationStatus);
	// All Effects와 같은 실제 파일 목록을 사용한다. Catalog 등록 여부는 목록 필터가 아닌
	// 각 Open/Preview/Append 소비자가 실행 시 확인한다.
	if (inventoryReady)
		for (const auto& row : inventory) m_SemanticValtanEffectAssetIds.push_back(row.strAssetId);
	else
		m_SemanticValtanEffectAssetIds.insert(m_SemanticValtanEffectAssetIds.end(), previousV1Ids.begin(), previousV1Ids.end());
	for (const auto& group : m_CinematicEffectLibrary.Groups)
		for (const auto& effect : group.Effects)
			if (effect.Key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT)
				m_SemanticValtanEffectAssetIds.push_back(effect.Key.strStableId);
	std::sort(m_SemanticValtanEffectAssetIds.begin(), m_SemanticValtanEffectAssetIds.end());
	m_SemanticValtanEffectAssetIds.erase(std::unique(m_SemanticValtanEffectAssetIds.begin(),
		m_SemanticValtanEffectAssetIds.end()), m_SemanticValtanEffectAssetIds.end());
	auto labels = m_EffectResourceLabels;
	for (const auto& id : m_SemanticValtanEffectAssetIds)
	{
		auto& label = labels[id];
		if (inventoryReady)
		{
			label.displayName = id;
			label.categories.clear();
			label.status.clear();
		}
		if (label.displayName.empty()) label.displayName = id;
	}
	if (inventoryReady)
		for (const auto& row : inventory)
			if (auto entry = labels.find(row.strAssetId); entry != labels.end())
			{
				if (!row.strDisplayName.empty()) entry->second.displayName = row.strDisplayName;
				entry->second.status = row.strStatus;
			}
	if (organizationReady)
		for (const auto& row : organization)
			if (auto entry = labels.find(row.strAssetId); entry != labels.end())
			{
				if (!row.strDisplayName.empty()) entry->second.displayName = row.strDisplayName;
				entry->second.categories = row.CategoryPath;
			}
	for (const auto& id : m_SemanticValtanEffectAssetIds)
	{
		auto& label = labels[id];
		const bool valtan = id.starts_with("effect.valtan.");
		if (restoreReady) label.fullRestore = restoreClips.contains(id);
		const bool restored = valtan && label.fullRestore;
		label.searchText = "V1 " + id + " " + label.displayName;
		label.patternCategories.clear();
		std::string patternName;
		size_t patternCount = 0u;
		bool combatObject = false;
		const auto joinPattern = [&](const VALTAN_PATTERN_VIEW& pattern)
		{
			const bool restoreMatch = restored && restoreReady && CValtanPatternTree::Matches_FullRestoreSource(pattern, id, restoreClips);
			bool productMatch = false;
			bool objectMatch = false;
			for (const auto& stage : pattern.Stages)
			{
				for (const auto& cue : stage.ProductCues)
					if (cue.strEffectAssetId == id || cue.strV1EffectAssetId == id) productMatch = true;
				for (const auto& object : stage.CombatObjectEffects)
					if (object.strEffectAssetId == id) { objectMatch = true; combatObject = true; }
			}
			if (!restoreMatch && !productMatch && !objectMatch) return;
			++patternCount;
			label.searchText += " " + pattern.strPatternId + " " + pattern.strDisplayName;
			const auto name = pattern.strDisplayName.empty() ? pattern.strPatternId : pattern.strDisplayName;
			if (patternName.empty()) patternName = name;
			if (restoreMatch) label.patternCategories.push_back({ "Patterns", name, "Full Restore" });
			if (productMatch || objectMatch) label.patternCategories.push_back({ "Patterns", name, "Product" });
		};
		for (const auto& pattern : m_CanonicalView.Gimmicks) joinPattern(pattern);
		for (const auto& pattern : m_CanonicalView.Rotation) joinPattern(pattern);
		bool independent = false;
		for (const auto& row : m_CanonicalView.IndependentEffects)
			if (row.strEffectAssetId == id)
			{
				independent = true;
				label.searchText += " " + row.strDisplayName + " " + row.strOwnerPatternId;
			}
		if (valtan)
		{
			if (independent || combatObject || patternCount > 1u)
				label.categories = { "Common", combatObject ? "Combat Objects" : restored ? "Full Restore" : "Effects" };
			else if (patternCount == 1u)
				label.categories = { "Patterns", patternName, restored ? "Full Restore" : "Product" };
			else if (!label.categories.empty())
			{
				if (label.categories.front() == "Valtan") label.categories.erase(label.categories.begin());
				if (label.categories.empty()) label.categories = { "Library", "Saved Effects" };
			}
			else label.categories = { "Library", restored ? "Full Restore" : "Saved Effects" };
			if (const auto source = restoreClips.find(id); source != restoreClips.end())
				for (const auto& clip : source->second) label.searchText += " " + clip.strClipName;
		}
		else if (label.categories.empty()) label.categories = { "Other Effects" };
		// 현재 Product binding이 없어도 재사용 가능한 원본 본문을 목록에서 찾을 수 있게 한다.
		if (id == "effect.valtan.source.fx_mn_rpbf_00_n.par_n_rpbf_dash_01_1" ||
			id == "effect.valtan.source.fx_mn_rpbf_00_s.par_s_rpbf_dash_01_1")
		{
			label.searchText += " VALTAN_DASH_CHARGE dash-charge";
			const auto joinSourceLibrary = [&](const VALTAN_PATTERN_VIEW& pattern)
			{
				if (pattern.strPatternId != "VALTAN_DASH_CHARGE") return;
				const auto name = pattern.strDisplayName.empty() ? pattern.strPatternId : pattern.strDisplayName;
				label.patternCategories.push_back({ "Patterns", name, "Source Library" });
				label.searchText += " " + name;
			};
			for (const auto& pattern : m_CanonicalView.Gimmicks) joinSourceLibrary(pattern);
			for (const auto& pattern : m_CanonicalView.Rotation) joinSourceLibrary(pattern);
		}
		bool firstCinematic = true;
		for (const auto& group : m_CinematicEffectLibrary.Groups)
		{
			if (std::none_of(group.Effects.begin(), group.Effects.end(), [&](const auto& effect)
				{ return effect.Key.eOwnerKind == EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT && effect.Key.strStableId == id; }))
				continue;
			const std::vector<std::string> path{ "Cinematics", group.strDisplayName };
			label.patternCategories.push_back(path);
			if (firstCinematic && patternCount == 0u && !independent && !combatObject)
				label.categories = path;
			firstCinematic = false;
			label.searchText += " " + group.strSequenceId + " " + group.strSourceDisplayName + " " + group.strPatternId;
			for (const auto& pattern : m_CanonicalView.Gimmicks)
				if (pattern.strPatternId == group.strPatternId) label.searchText += " " + pattern.strDisplayName;
			label.status += (label.status.empty() ? "" : " | ") + std::string("Cinematic: ") + group.strDisplayName;
		}
		for (const auto& category : label.categories) label.searchText += " " + category;
		for (const auto& path : label.patternCategories)
			for (const auto& category : path) label.searchText += " " + category;
	}
	m_EffectResourceLabels = std::move(labels);

	std::string V2Status;
	const bool_t bV2Reloaded = !bReloadCatalog ||
		CEffectV2Catalog::Get().Reload_BossValtan(V2Status);
	const std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> pV2Snapshot =
		CEffectV2Catalog::Get().Get_Snapshot();
	if (nullptr != pV2Snapshot && pV2Snapshot->Is_Ready())
	{
		const auto describeTypedEffect = [&](const std::string& id, const std::string& name)
		{
			auto& label = m_EffectResourceLabels[id];
			label.displayName = name.empty() ? id : name;
			label.searchText = "V2 " + id + " " + name;
			label.status.clear();
			label.patternCategories.clear();
			size_t patternCount = 0u;
			bool combatObject = false;
			std::string patternName;
			const auto join = [&](const VALTAN_PATTERN_VIEW& pattern)
			{
				bool used = std::any_of(pV2Snapshot->Get_BossValtanBindings().begin(), pV2Snapshot->Get_BossValtanBindings().end(),
					[&](const auto& row) { return row.strResourceId == id && row.strPatternId == pattern.strPatternId; });
				for (const auto& stage : pattern.Stages)
					for (const auto& object : stage.CombatObjectEffects)
						if (object.strEffectV2GroupId == id) { used = true; combatObject = true; }
				if (used)
				{
					++patternCount;
					label.searchText += " " + pattern.strPatternId + " " + pattern.strDisplayName;
					const auto name = pattern.strDisplayName.empty() ? pattern.strPatternId : pattern.strDisplayName;
					if (patternName.empty()) patternName = name;
					label.patternCategories.push_back({ "Patterns", name, "Product" });
				}
			};
			for (const auto& pattern : m_CanonicalView.Gimmicks) join(pattern);
			for (const auto& pattern : m_CanonicalView.Rotation) join(pattern);
			if (!id.starts_with("boss.valtan.")) label.categories = { "Other Effects" };
			else if (combatObject || patternCount > 1u) label.categories = { "Common", combatObject ? "Combat Objects" : "Effects" };
			else if (patternCount == 1u) label.categories = { "Patterns", patternName, "Product" };
			else label.categories = { "Library", "Saved Effects" };
			for (const auto& category : label.categories) label.searchText += " " + category;
		};
		m_EffectV2DocumentIds.clear();
		m_EffectV2DocumentTypes.clear();
		m_EffectV2GroupIds.clear();
		m_EffectV2DocumentIds.reserve(
			pV2Snapshot->Get_Documents().size());
		m_EffectV2DocumentTypes.reserve(
			pV2Snapshot->Get_Documents().size());
		for (const EFFECT_V2_DOCUMENT& Document :
			pV2Snapshot->Get_Documents())
		{
			m_EffectV2DocumentIds.push_back(Document.strEffectId);
			describeTypedEffect(Document.strEffectId, Document.strDisplayName);
			m_EffectV2DocumentTypes.push_back(
				static_cast<int32_t>(Document.eType));
		}
		m_EffectV2GroupIds.reserve(pV2Snapshot->Get_Groups().size());
		for (const EFFECT_V2_GROUP& Group : pV2Snapshot->Get_Groups())
		{
			m_EffectV2GroupIds.push_back(Group.strGroupId);
			describeTypedEffect(Group.strGroupId, Group.strDisplayName);
		}
		m_iEffectV2CatalogRevision = pV2Snapshot->Get_Revision();
	}
	else
	{
		m_EffectV2DocumentIds.clear();
		m_EffectV2DocumentTypes.clear();
		m_EffectV2GroupIds.clear();
		m_iEffectV2CatalogRevision = 0u;
	}
	std::vector<EFFECT_V2_RESOURCE_SUMMARY> typedInventory;
	std::string typedInventoryStatus;
	const bool typedInventoryReady = CEffectV2Catalog::Get().Read_Inventory(typedInventory, typedInventoryStatus);
	if (typedInventoryReady)
		for (const auto& row : typedInventory)
		{
			auto& ids = EFFECT_V2_RESOURCE_KIND::GROUP == row.eKind ? m_EffectV2GroupIds : m_EffectV2DocumentIds;
			if (ids.end() == std::find(ids.begin(), ids.end(), row.strResourceId))
			{
				ids.push_back(row.strResourceId);
				if (EFFECT_V2_RESOURCE_KIND::LEAF == row.eKind)
					m_EffectV2DocumentTypes.push_back(static_cast<int32_t>(EFFECT_V2_TYPE::END));
				auto& label = m_EffectResourceLabels[row.strResourceId];
				label.categories = { row.strResourceId.starts_with("boss.valtan.") ? "Library" : "Other Effects", "Saved Effects" };
				label.patternCategories.clear();
			}
			auto& label = m_EffectResourceLabels[row.strResourceId];
			label.displayName = row.strDisplayName.empty() ? row.strResourceId : row.strDisplayName;
			label.searchText += " V2 " + row.strResourceId + " " + label.displayName + " " + row.strCategory;
			label.status = row.strStatus;
		}
	std::unordered_map<std::string, std::string> friendlySavedNames;
	for (const auto& row : inventory) friendlySavedNames[row.strAssetId] = row.strDisplayName;
	for (const auto& row : typedInventory) friendlySavedNames[row.strResourceId] = row.strDisplayName;
	CValtanPatternTree::Build_EffectResourceDisplayLabels(m_CanonicalView, friendlySavedNames, restoreClips);
	for (auto& [id, label] : m_EffectResourceLabels)
	{
		label.displayName = CValtanPatternTree::Describe_EffectResource(
			m_CanonicalView, id, label.displayName, restoreClips);
		label.searchText += " " + label.displayName;
	}
	m_strEffectCatalogStatus =
		"V1 authored " + std::to_string(m_SemanticValtanEffectAssetIds.size()) +
		" | BOSS_VALTAN V2 group " +
			std::to_string(m_EffectV2GroupIds.size()) +
		" | advanced leaf " + std::to_string(m_EffectV2DocumentIds.size());
	if (!bV2Reloaded && !V2Status.empty())
		m_strEffectCatalogStatus += " | V2 reload preserved previous snapshot: " +
			V2Status;

	if (!m_strCinematicEffectLibraryStatus.empty())
		m_strEffectCatalogStatus += " | " + m_strCinematicEffectLibraryStatus;
	if (!restoreReady) m_strEffectCatalogStatus += " | Full Restore: " + restoreStatus;
	if (!inventoryReady) m_strEffectCatalogStatus += " | Saved names: " + inventoryStatus;
	if (!organizationReady) m_strEffectCatalogStatus += " | Categories: " + organizationStatus;
	if (!typedInventoryReady) m_strEffectCatalogStatus += " | V2 inventory: " + typedInventoryStatus;

	bool_t bSelectedExists = m_strEffectAddAssetId.empty();
	if (!bSelectedExists)
	{
		const std::vector<std::string>* pOwner = nullptr;
		switch (m_eEffectAddResourceKind)
		{
		case EFFECT_RESOURCE_KIND::V1_PATTERN:
			pOwner = &m_SemanticValtanEffectAssetIds;
			break;
		case EFFECT_RESOURCE_KIND::V2_LEAF:
			pOwner = &m_EffectV2DocumentIds;
			break;
		case EFFECT_RESOURCE_KIND::V2_GROUP:
			pOwner = &m_EffectV2GroupIds;
			break;
		default:
			break;
		}
		bSelectedExists = nullptr != pOwner && pOwner->end() != std::find(
			pOwner->begin(), pOwner->end(), m_strEffectAddAssetId);
	}
	if (!bSelectedExists)
	{
		m_strEffectAddAssetId.clear();
	}
	Invalidate_TimelineCache();
}

void Client::CValtanActionWorkbench::Reset_EffectCueEditor()
{
	m_strEffectEditIdentity.clear();
	m_EffectCueEditDraft = {};
	m_strEffectAddClipOccurrenceId.clear();
}

bool_t Client::CValtanActionWorkbench::Open_Valtan()
{
	m_bPatternsWindowVisible = true;
	m_bPreviewWindowVisible = true;
	m_bSequencerWindowVisible = true;
	m_bDetailsWindowVisible = true;
	m_bResourcesWindowVisible = true;
	if (!m_bLoadAttempted || !Can_MutateValtanView(m_eAdmission))
		return Reload_Canonical();
	return true;
}

bool_t Client::CValtanActionWorkbench::Ensure_CanonicalLoaded(
	std::string& strOutStatus)
{
	if (m_bLoadAttempted && Can_MutateValtanView(m_eAdmission))
		return true;
	if (!Reload_Canonical())
	{
		strOutStatus = m_strStatus;
		return false;
	}
	return true;
}

std::vector<std::string>
Client::CValtanActionWorkbench::Get_PatternIds() const
{
	std::vector<std::string> Ids;
	for (const VALTAN_PATTERN_VIEW* const pPattern : Collect_Patterns())
		Ids.push_back(pPattern->strPatternId);
	return Ids;
}

bool_t Client::CValtanActionWorkbench::Select_PatternById(
	const std::string& strPatternId,
	std::string& strOutStatus)
{
	if (nullptr != m_pBalanceTool &&
		(m_pBalanceTool->Is_ValtanSaveJobBlockingAuthoring() ||
		 m_pBalanceTool->Is_ServerRuntimeSetPublishRunning()))
	{
		strOutStatus =
			"Pattern selection is blocked until the exact Save/publish receipt is terminal.";
		return false;
	}
	if (!Ensure_CanonicalLoaded(strOutStatus))
		return false;
	const VALTAN_PATTERN_VIEW* const pPattern = Find_PatternById(strPatternId);
	if (nullptr == pPattern)
	{
		strOutStatus =
			"Pattern is not in the playable Composition inventory: " + strPatternId;
		return false;
	}
	if (pPattern->strPatternId != m_strSelectedPatternId)
	{
		bool_t bAuthoringDirty = false;
		std::string Revision;
		std::string AuthoringStatus;
		if (nullptr != m_pBalanceTool)
		{
			(void)m_pBalanceTool->Get_ValtanAuthoringState(
				Revision, bAuthoringDirty, AuthoringStatus);
		}
		std::string SoundStatus;
		if (bAuthoringDirty || Is_PatternSoundDraftDirty(SoundStatus) || m_CombatObjectSounds.bDraftDirty ||
			CEffectV2Catalog::Get().Has_BossValtanBindingDraft())
		{
			strOutStatus =
				"Save or discard the Composition draft in the Workbench before selecting another Pattern from the Sequencer.";
			return false;
		}
		Select_Pattern(*pPattern);
	}
	return Ensure_SelectedTimeline(strOutStatus);
}

bool_t Client::CValtanActionWorkbench::Ensure_SelectedTimeline(
	std::string& strOutStatus)
{
	const VALTAN_PATTERN_VIEW* const pPattern =
		Find_PatternById(m_strSelectedPatternId);
	if (nullptr == pPattern)
	{
		strOutStatus = "No Composition Pattern is selected.";
		return false;
	}
	Ensure_TimelineCache(pPattern);
	return true;
}

void Client::CValtanActionWorkbench::On_LevelChanged()
{
	m_PendingLocalPreview.reset();
	m_bWorkbenchBossLoadAttempted = false;
	m_PendingResourceAppend.reset();
	m_PendingCompositionPaste.reset();
	/* canonical 데이터는 Level 전환 뒤에도 남지만 모델 자세와 Server occurrence는 같은 수명을 갖지 않는다.
	   의미상의 선택 ID는 보존하고 preview 상태는 다시 조회한다. */
	m_iPlayheadMs = 0u;
}

const char_t* Client::CValtanActionWorkbench::Admission_Label() const
{
	switch (m_eAdmission)
	{
	case VALTAN_VIEW_ADMISSION::UNLOADED:
		return "NOT LOADED";
	case VALTAN_VIEW_ADMISSION::ADMITTED:
		return "ADMITTED";
	case VALTAN_VIEW_ADMISSION::STALE_PRESERVED:
		return "STALE PRESERVED / READ ONLY";
	case VALTAN_VIEW_ADMISSION::REJECTED:
		return "REJECTED";
	default:
		return "INVALID";
	}
}

const char_t* Client::CValtanActionWorkbench::Owner_Label(
	const DETAIL_OWNER eOwner)
{
	switch (eOwner)
	{
	case DETAIL_OWNER::PATTERN: return "Pattern Root / Flow";
	case DETAIL_OWNER::GAMEPLAY_STAGE: return "Gameplay / Logic / Collider";
	case DETAIL_OWNER::ANIMATION: return "Animation Sequence";
	case DETAIL_OWNER::EFFECT: return "Effect";
	case DETAIL_OWNER::SOUND: return "Sound";
	case DETAIL_OWNER::CAMERA: return "Camera / Light";
	case DETAIL_OWNER::WORLD: return "World";
	case DETAIL_OWNER::SCENE_PROFILE: return "Scene Profile";
	case DETAIL_OWNER::LIGHT: return "Light";
	case DETAIL_OWNER::COMBAT_OBJECT: return "Combat Object";
	default: return "Unknown";
	}
}

const char_t* Client::CValtanActionWorkbench::Lane_Label(
	const TIMELINE_LANE eLane)
{
	switch (eLane)
	{
	case TIMELINE_LANE::STAGE: return "Stages";
	case TIMELINE_LANE::ANIMATION: return "Animation";
	case TIMELINE_LANE::EFFECT: return "Effect";
	case TIMELINE_LANE::SOUND: return "Sound";
	case TIMELINE_LANE::LOGIC: return "Logic";
	case TIMELINE_LANE::COLLIDER: return "Collider";
	case TIMELINE_LANE::CAMERA: return "Camera";
	case TIMELINE_LANE::WORLD: return "World";
	case TIMELINE_LANE::SUMMON: return "Summon";
	case TIMELINE_LANE::SCENE_PROFILE: return "Scene Profile";
	case TIMELINE_LANE::LIGHT: return "Light";
	default: return "Invalid";
	}
}

uint32_t Client::CValtanActionWorkbench::Lane_Color(
	const TIMELINE_LANE eLane)
{
	switch (eLane)
	{
	case TIMELINE_LANE::STAGE: return CompositionTimeline::StageColor;
	case TIMELINE_LANE::ANIMATION: return CompositionTimeline::AnimationColor;
	case TIMELINE_LANE::EFFECT: return CompositionTimeline::PresentationColor;
	case TIMELINE_LANE::SOUND: return CompositionTimeline::PresentationColor;
	case TIMELINE_LANE::LOGIC: return CompositionTimeline::LogicColor;
	case TIMELINE_LANE::COLLIDER: return CompositionTimeline::PresentationColor;
	case TIMELINE_LANE::CAMERA: return CompositionTimeline::PresentationColor;
	case TIMELINE_LANE::WORLD: return CompositionTimeline::WorldColor;
	case TIMELINE_LANE::SUMMON: return CompositionTimeline::SummonColor;
	case TIMELINE_LANE::SCENE_PROFILE: return CompositionTimeline::SceneProfileColor;
	case TIMELINE_LANE::LIGHT: return CompositionTimeline::PresentationColor;
	default: return IM_COL32(116, 126, 138, 255);
	}
}

// Browser 표시를 위해 저작 관리 Pattern의 포인터 목록만 만든 뒤 phase·gimmick·ID 순으로 안정 정렬한다.
// 원본 Pattern vector는 재배치하지 않는다. 반환 포인터는 canonical 저장소가 교체되기 전까지만 유효하다.
std::vector<const Client::VALTAN_PATTERN_VIEW*>
Client::CValtanActionWorkbench::Collect_Patterns() const
{
	std::vector<const VALTAN_PATTERN_VIEW*> Patterns;
	Patterns.reserve(m_CanonicalView.Get_PatternCount());
	for (const VALTAN_PATTERN_VIEW& Pattern : m_CanonicalView.Gimmicks)
		if (Pattern.bAuthoringMasterManaged)
			Patterns.push_back(&Pattern);
	for (const VALTAN_PATTERN_VIEW& Pattern : m_CanonicalView.Rotation)
		if (Pattern.bAuthoringMasterManaged)
			Patterns.push_back(&Pattern);
	std::stable_sort(
		Patterns.begin(), Patterns.end(),
		[](const VALTAN_PATTERN_VIEW* const pLeft,
			const VALTAN_PATTERN_VIEW* const pRight)
		{
			if (pLeft->iMinimumPhase != pRight->iMinimumPhase)
				return pLeft->iMinimumPhase < pRight->iMinimumPhase;
			if (pLeft->Is_Gimmick() != pRight->Is_Gimmick())
				return pLeft->Is_Gimmick() > pRight->Is_Gimmick();
			return pLeft->strPatternId < pRight->strPatternId;
		});
	return Patterns;
}

std::vector<const Client::VALTAN_PATTERN_VIEW*>
Client::CValtanActionWorkbench::
Collect_CanonicalPatternsForDependencyValidation() const
{
	/* Sound 종속 검증은 Browser의 bAuthoringMasterManaged 필터와 달리 전체 canonical 목록을 모은다.
	   호환 행의 track도 검사 대상에 포함하며 Server Complete Play 허용 여부와는 구분한다. */
	std::vector<const VALTAN_PATTERN_VIEW*> Patterns;
	Patterns.reserve(m_CanonicalView.Get_PatternCount());
	for (const VALTAN_PATTERN_VIEW& Pattern : m_CanonicalView.Gimmicks)
		Patterns.push_back(&Pattern);
	for (const VALTAN_PATTERN_VIEW& Pattern : m_CanonicalView.Rotation)
		Patterns.push_back(&Pattern);
	return Patterns;
}

void Client::CValtanActionWorkbench::
Invalidate_SourceSequenceOwnerIndex()
{
	m_SourceSequenceOwnerIndex.clear();
	m_iSourceSequenceOwnerIndexGeneration = ~std::uint64_t{ 0u };
	m_strSourceSequenceServerPatternId.clear();
}

// source action ID와 sequence index가 같은 항목에 Pattern 포인터를 모으고 정렬한다.
// 구축 중에는 선형 검색으로 묶으므로 대규모 입력에서 비용이 늘 수 있지만 매 frame이 아니라 canonical display generation 변경 때 다시 만든다.
// 조회는 아래 lower_bound의 정렬 전제를 사용하며 이 인덱스 자체가 Pattern 값을 소유하지 않는다.
void Client::CValtanActionWorkbench::Ensure_SourceSequenceOwnerIndex()
{
	if (m_iSourceSequenceOwnerIndexGeneration ==
		m_iCanonicalDisplayGeneration)
	{
		return;
	}

	std::vector<SOURCE_SEQUENCE_OWNER_INDEX_ENTRY> Staged;
	const std::vector<const VALTAN_PATTERN_VIEW*> Patterns = Collect_Patterns();
	Staged.reserve(Patterns.size());
	for (const VALTAN_PATTERN_VIEW* const pPattern : Patterns)
	{
		if (nullptr == pPattern)
			continue;
		for (const VALTAN_PRESENTATION_SOURCE_VIEW& Source :
			pPattern->PresentationSources)
		{
			/* REFERENCE 행은 조사·출처 연결이다.
			   해당 Product Pattern이 이 원본 Sequence를 직접 소유해 Server Play할 수 있다는 뜻이 아니다. */
			if ("PRIMARY" != Source.strRole)
				continue;
			auto Entry = std::find_if(
				Staged.begin(), Staged.end(),
				[&Source](const SOURCE_SEQUENCE_OWNER_INDEX_ENTRY& Candidate)
				{
					return Candidate.iSourceActionId ==
							Source.iSourceActionId &&
						Candidate.iSequenceIndex == Source.iSequenceIndex;
				});
			if (Entry == Staged.end())
			{
				SOURCE_SEQUENCE_OWNER_INDEX_ENTRY Candidate;
				Candidate.iSourceActionId = Source.iSourceActionId;
				Candidate.iSequenceIndex = Source.iSequenceIndex;
				Staged.push_back(std::move(Candidate));
				Entry = Staged.end() - 1;
			}
			if (Entry->Owners.end() == std::find(
					Entry->Owners.begin(), Entry->Owners.end(), pPattern))
			{
				Entry->Owners.push_back(pPattern);
			}
		}
	}
	std::sort(
		Staged.begin(), Staged.end(),
		[](const SOURCE_SEQUENCE_OWNER_INDEX_ENTRY& Left,
			const SOURCE_SEQUENCE_OWNER_INDEX_ENTRY& Right)
		{
			if (Left.iSourceActionId != Right.iSourceActionId)
				return Left.iSourceActionId < Right.iSourceActionId;
			return Left.iSequenceIndex < Right.iSequenceIndex;
		});
	m_SourceSequenceOwnerIndex = std::move(Staged);
	m_iSourceSequenceOwnerIndexGeneration = m_iCanonicalDisplayGeneration;
}

// 정렬 기준인 두 32bit 값을 64bit key의 상위·하위로 결합한다.
// lower_bound 뒤에도 두 필드를 확인해 삽입 위치를 일치 항목으로 오인하지 않는다. 반환은 인덱스 vector 안의 차용 포인터다.
const Client::CValtanActionWorkbench::
	SOURCE_SEQUENCE_OWNER_INDEX_ENTRY*
Client::CValtanActionWorkbench::Find_SourceSequenceOwners(
	const uint32_t iSourceActionId,
	const uint32_t iSequenceIndex) const
{
	const std::uint64_t iKey =
		(static_cast<std::uint64_t>(iSourceActionId) << 32u) |
		static_cast<std::uint64_t>(iSequenceIndex);
	const auto Found = std::lower_bound(
		m_SourceSequenceOwnerIndex.begin(),
		m_SourceSequenceOwnerIndex.end(), iKey,
		[](const SOURCE_SEQUENCE_OWNER_INDEX_ENTRY& Entry,
			const std::uint64_t iCandidateKey)
		{
			const std::uint64_t iEntryKey =
				(static_cast<std::uint64_t>(Entry.iSourceActionId) << 32u) |
				static_cast<std::uint64_t>(Entry.iSequenceIndex);
			return iEntryKey < iCandidateKey;
		});
	if (Found == m_SourceSequenceOwnerIndex.end() ||
		Found->iSourceActionId != iSourceActionId ||
		Found->iSequenceIndex != iSequenceIndex)
	{
		return nullptr;
	}
	return &*Found;
}

const Client::VALTAN_PATTERN_VIEW*
Client::CValtanActionWorkbench::Find_PatternById(
	const std::string& strPatternId) const
{
	/* ID로 조회할 때는 호환 Pattern의 저장 track도 찾을 수 있다.
	   편집은 승인된 분리 source 소유권을 사용하며 Browser 표시 필터와 Server Play 허용 목록은 별도로 적용한다. */
	const auto Match = [&strPatternId](const VALTAN_PATTERN_VIEW& Pattern)
	{
		return Pattern.strPatternId == strPatternId;
	};
	if (const auto Found = std::find_if(
			m_CanonicalView.Gimmicks.begin(), m_CanonicalView.Gimmicks.end(), Match);
		Found != m_CanonicalView.Gimmicks.end())
	{
		return &*Found;
	}
	if (const auto Found = std::find_if(
			m_CanonicalView.Rotation.begin(), m_CanonicalView.Rotation.end(), Match);
		Found != m_CanonicalView.Rotation.end())
	{
		return &*Found;
	}
	return nullptr;
}

const Client::VALTAN_PATTERN_VIEW*
Client::CValtanActionWorkbench::Find_SelectedPattern() const
{
	return Find_PatternById(m_strSelectedPatternId);
}

// Pattern 포인터는 선택이 없을 때 nullptr를 표현한다. null을 먼저 거르고 선택 Stage ID로 원본 vector를 조회한다.
// 결과 역시 전달받은 Pattern 안의 차용 주소이므로 Pattern 교체 뒤에는 다시 찾아야 한다.
const Client::VALTAN_STAGE_VIEW*
Client::CValtanActionWorkbench::Find_SelectedStage(
	const VALTAN_PATTERN_VIEW* const pPattern) const
{
	if (nullptr == pPattern)
		return nullptr;
	const auto Found = std::find_if(
		pPattern->Stages.begin(), pPattern->Stages.end(),
		[this](const VALTAN_STAGE_VIEW& Stage)
		{
			return Stage.strStageId == m_strSelectedStageId;
		});
	return Found == pPattern->Stages.end() ? nullptr : &*Found;
}

void Client::CValtanActionWorkbench::Normalize_Selection()
{
	const VALTAN_PATTERN_VIEW* pPattern = Find_SelectedPattern();
	bool_t bPatternSelectionChanged = false;
	if (nullptr == pPattern)
	{
		/* 선택 stable ID가 없거나 거절됐을 때만 목록을 모아 정렬한다.
		   유효한 선택을 가진 프레임은 이 분기에 들어오지 않아 이곳의 Pattern 포인터 목록을 다시 만들지 않는다. */
		const std::vector<const VALTAN_PATTERN_VIEW*> Patterns =
			Collect_Patterns();
		if (Patterns.empty())
		{
			if (m_bProductFallbackReady)
			{
				const auto& ProductPatterns =
					m_ProductFallbackEncounter.Get_Patterns();
				const ENCOUNTER_PATTERN_REFERENCE* pFallback =
					m_ProductFallbackEncounter.Find_Pattern(
						m_strSelectedPatternId);
				if (nullptr == pFallback && !ProductPatterns.empty())
					pFallback = &ProductPatterns.front();
				m_strSelectedPatternId = nullptr == pFallback ?
					std::string{} : pFallback->patternId;
				if (nullptr == pFallback || pFallback->stages.empty())
					m_strSelectedStageId.clear();
				else if (pFallback->stages.end() == std::find_if(
						pFallback->stages.begin(), pFallback->stages.end(),
						[this](const ENCOUNTER_STAGE_REFERENCE& Stage)
						{ return Stage.stageId == m_strSelectedStageId; }))
				{
					m_strSelectedStageId =
						pFallback->stages.front().stageId;
				}
				m_strSelectedStableId = m_strSelectedPatternId;
				m_TimelineItems.clear();
				m_iTimelineDurationMs = 0u;
				return;
			}
			m_strSelectedPatternId.clear();
			m_strSelectedStageId.clear();
			m_strSelectedStableId.clear();
			m_TimelineItems.clear();
			m_iTimelineDurationMs = 0u;
			return;
		}
		pPattern = Patterns.front();
		m_strSelectedPatternId = pPattern->strPatternId;
		m_ePreviewPath = InitialPreviewPath(*pPattern);
		bPatternSelectionChanged = true;
		m_eDetailOwner = DETAIL_OWNER::PATTERN;
		m_strSelectedStableId = pPattern->strPatternId;
	}
	if (pPattern->Stages.empty())
		m_strSelectedStageId.clear();
	else if (bPatternSelectionChanged || m_strSelectedStageId.empty())
		m_strSelectedStageId = pPattern->Stages.front().strStageId;
	m_iPlayheadMs = (std::min)(m_iPlayheadMs, m_iTimelineDurationMs);
}

// 저장 작업과 미저장 draft를 확인한 뒤 canonical 읽기 승인을 얻고 임시 view를 준비한다.
// 로드 실패는 이전 표시를 보존하되 변경 승인을 제한한다. 표시 성공과 source 편집 가능·Server Product 재생 가능은 서로 다른 판정이다.
bool_t Client::CValtanActionWorkbench::Reload_Canonical(
	const bool_t bReuseCommittedSource)
{
	Engine::CProfilerScope reloadScope(
		CGameInstance::Get().Get_Profiler(), "Tool.Composition.ReloadCanonical");
	if (nullptr != m_pBalanceTool &&
		(m_pBalanceTool->Is_ValtanSaveJobBlockingAuthoring() ||
		 m_pBalanceTool->Is_ServerRuntimeSetPublishRunning()))
	{
		m_strStatus =
			"Canonical reload blocked until the exact Save/publish receipt is terminal; the current snapshot was preserved.";
		return false;
	}
	m_bLoadAttempted = true;
	m_bCanonicalReloadRetryPending = false;
	Invalidate_EffectivePatternCache();
	if (m_CombatObjectSounds.bDraftDirty)
	{
		m_strStatus = "Canonical reload blocked: save or explicitly discard the unsaved Object Sound draft.";
		return false;
	}
	std::string SoundDraftStatus;
	if (Is_PatternSoundDraftDirty(SoundDraftStatus))
	{
		m_strStatus =
			"Canonical reload blocked: save or explicitly discard the unsaved Pattern Sound owner draft before changing its dependency generation. " +
			SoundDraftStatus;
		return false;
	}
	std::string AuthoringRevision;
	std::string CanonicalSourceRevision;
	bool_t bAuthoringDirty = false;
	const bool_t bBalanceDraftDirty = nullptr != m_pBalanceTool &&
		m_pBalanceTool->Is_ValtanDraftDirty();
	const bool_t bHasDisplaySnapshot =
		0u != m_CanonicalView.Get_PatternCount();
	if (bBalanceDraftDirty && bHasDisplaySnapshot)
	{
		m_strStatus =
			"Canonical reload skipped because the admitted effective authoring draft has unsaved changes. Save it first; the current admitted draft and read-only Product display were preserved.";
		return false;
	}
	CValtanCanonicalProductReadAdmission CanonicalAdmission;
	VALTAN_CANONICAL_READ_DIAGNOSTIC CanonicalDiagnostic;
	if (!CanonicalAdmission.Acquire(CanonicalDiagnostic))
	{
		const bool_t bPreserved = 0u != m_CanonicalView.Get_PatternCount();
		m_bCanonicalReloadRetryPending =
			CanonicalDiagnostic.Is_AutomaticRetryable();
		if (m_bCanonicalReloadRetryPending)
		{
			m_dNextCanonicalReloadRetrySeconds =
				ImGui::GetTime() + CANONICAL_RELOAD_RETRY_SECONDS;
		}
		m_eAdmission = bPreserved ? VALTAN_VIEW_ADMISSION::STALE_PRESERVED :
			VALTAN_VIEW_ADMISSION::REJECTED;
		if (m_bCanonicalReloadRetryPending)
		{
			m_strStatus = bPreserved ?
				"Canonical reload admission was busy; previous Workbench data is read-only and reload will retry automatically: " +
					CanonicalDiagnostic.strStatus :
				"Canonical reload admission is temporarily busy; Pattern / Stage Browser will retry automatically: " +
					CanonicalDiagnostic.strStatus;
		}
		else
		{
			m_strStatus = bPreserved ?
				"Canonical reload admission failed; previous Workbench data is preserved read-only: " +
					CanonicalDiagnostic.strStatus :
				(m_bProductFallbackReady ?
					"READ-ONLY PRODUCT FALLBACK STALE_PRESERVED; canonical admission failed, so no unpinned Product files were reopened: " +
						CanonicalDiagnostic.strStatus :
					"Canonical reload admission failed; no mutation or Server playback is available and no unpinned fallback read was attempted: " +
						CanonicalDiagnostic.strStatus);
		}
		return false;
	}
	// Product 재생은 Boss Tool이 계속 담당한다. 편집기는 아직 새 runtime generation을 만들 준비가 안 된
	// source 행도 열 수 있다.
	VALTAN_PATTERN_TREE_VIEW Staged;
	if (!bReuseCommittedSource)
	{
		m_bProductSourceReady = CValtanPatternTree::Load_WhileAdmitted(
			CanonicalAdmission, Staged, CanonicalDiagnostic);
		m_strProductReadiness = CanonicalDiagnostic.strStatus;
	}
	else
	{
		// source Save는 Product를 교체하지 않았다. 정확한 Balance 소유 도구가 Save 승인 아래 이미 다시 열었으므로
		// 아래에서 revision을 검증한 뒤 사용한다.
		m_bProductSourceReady = false;
		m_strProductReadiness = "Source saved; Publish is required to refresh Product.";
	}
	std::string Diagnostic;
	const bool_t bSourceLoaded = bReuseCommittedSource ?
		(nullptr != m_pBalanceTool && m_pBalanceTool->Get_ValtanAuthoringView(Staged, Diagnostic)) :
		CValtanPatternTree::Load_Authoring_WhileAdmitted(CanonicalAdmission, Staged, Diagnostic);
	if (!bSourceLoaded)
	{
		m_eAdmission = m_CanonicalView.Get_PatternCount() ?
			VALTAN_VIEW_ADMISSION::STALE_PRESERVED : VALTAN_VIEW_ADMISSION::REJECTED;
		m_strStatus = "Source reload failed; the previous editing view was preserved: " + Diagnostic;
		return false;
	}
	VALTAN_TOOL_AUDITION_INVENTORY Inventory;
	std::string InventoryStatus;
	if (!CValtanPatternTree::Build_PlayablePatternInventory(
			Staged, Inventory, InventoryStatus))
	{
		const bool_t bPreserved = 0u != m_CanonicalView.Get_PatternCount();
		m_eAdmission = bPreserved ?
			VALTAN_VIEW_ADMISSION::STALE_PRESERVED : VALTAN_VIEW_ADMISSION::REJECTED;
		if (bPreserved)
		{
			m_strStatus =
				"Playable inventory rejected; previous graph is displayed read-only: " +
				InventoryStatus;
			Normalize_Selection();
		}
		else
		{
			(void)Stage_ProductFallback(
				CanonicalAdmission, InventoryStatus);
		}
		return false;
	}
	const auto PreserveOrCommitReadOnlyProduct =
		[this, &CanonicalAdmission, &Staged, &Diagnostic, &Inventory](
			const std::string& Cause) -> bool_t
	{
		VALTAN_PATTERN_SHAKE_CUE_DOCUMENT StagedPatternShakes;
		VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT StagedCombatObjectSounds;
		std::string StagedShakeStatus;
		std::string StagedCombatObjectSoundStatus;
		const bool_t bStagedPatternShakesReady =
			CValtanPatternShakeCueDocument::Load_Source(
				StagedPatternShakes, StagedShakeStatus);
		const bool_t bStagedCombatObjectSoundsReady =
			CValtanCombatObjectSoundCueDocument::Load_Source(
				StagedCombatObjectSounds, StagedCombatObjectSoundStatus);
		std::string CurrentStatus;
		if (!CanonicalAdmission.Validate_StillCurrent(CurrentStatus))
		{
			const bool_t bPreserved =
				0u != m_CanonicalView.Get_PatternCount();
			m_eAdmission = bPreserved ? VALTAN_VIEW_ADMISSION::STALE_PRESERVED :
				VALTAN_VIEW_ADMISSION::REJECTED;
			m_strStatus = bPreserved ?
				"Product/source join failed and the candidate Product generation changed before display commit; the previous display snapshot was preserved: " +
					CurrentStatus + " " + Cause :
				"Product/source join failed and no stable Product display generation was available: " +
					CurrentStatus + " " + Cause;
			return false;
		}

		const bool_t bPreserved = 0u != m_CanonicalView.Get_PatternCount();
		if (!bPreserved)
		{
			m_CanonicalView = std::move(Staged);
			m_PlayableInventory = Inventory;
			m_ProductFallbackEncounter.Clear();
			m_bProductFallbackReady = false;
			++m_iCanonicalDisplayGeneration;
			Invalidate_SourceSequenceOwnerIndex();
			m_strPinnedAuthoringSourceRevision.clear();
			m_strPinnedCanonicalSourceRevision.clear();
			m_bAuthoringDraftDirty = false;
			m_bPatternSoundDependencyDirty = false;
			m_PatternSoundEvents.clear();
			m_PatternShakes = std::move(StagedPatternShakes);
			m_bPatternShakesReady = bStagedPatternShakesReady;
			m_strShakeStatus = std::move(StagedShakeStatus);
			m_CombatObjectSounds = std::move(StagedCombatObjectSounds);
			m_bCombatObjectSoundsReady =
				bStagedCombatObjectSoundsReady;
			m_strCombatObjectSoundStatus =
				std::move(StagedCombatObjectSoundStatus);
			m_strDisplayProvenance =
				"PRODUCT_ONLY / canonical Product read admission; typed authoring and Pattern Sound owners are not joined.";
			Reset_EffectCueEditor();
			m_SemanticValtanEffectAssetIds.clear();
			m_bSemanticValtanEffectLoadAttempted = false;
			Invalidate_TimelineCache();
			Normalize_Selection();
		}
		m_eAdmission = VALTAN_VIEW_ADMISSION::STALE_PRESERVED;
		m_bConfirmDiscardPatternSoundDraft = false;
		m_strStatus = bPreserved ?
			"Typed source-owner join failed; the previous display snapshot was preserved read-only without mixing generations. Provenance: " +
				m_strDisplayProvenance + " Cause: " + Cause :
			"Canonical Product display loaded read-only: " +
				std::to_string(m_CanonicalView.Get_PatternCount()) +
				" patterns / " +
				std::to_string(m_CanonicalView.Get_StageCount()) +
				" stages / " +
				std::to_string(Inventory.Get_PatternCount()) +
				" Complete Play inventory rows. Provenance: " +
				m_strDisplayProvenance + " Cause: " + Cause + " " + Diagnostic;
		return false;
	};
	if (bBalanceDraftDirty)
	{
		return PreserveOrCommitReadOnlyProduct(
			"Balance Tool has unsaved Valtan changes. The draft was left untouched; only the last saved canonical Product was staged for read-only browsing.");
	}

	std::string AuthoringStatus;
	if (nullptr == m_pBalanceTool ||
		(!bReuseCommittedSource && !m_pBalanceTool->Reload_ValtanSource(AuthoringStatus)) ||
		!m_pBalanceTool->Get_ValtanAuthoringState(
			AuthoringRevision, bAuthoringDirty, AuthoringStatus) ||
		!m_pBalanceTool->Get_ValtanCanonicalSourceRevision(
			CanonicalSourceRevision, AuthoringStatus))
	{
		return PreserveOrCommitReadOnlyProduct(
			"Typed authoring source reload/join rejected: " + AuthoringStatus);
	}
	if (!m_pBalanceTool->Verify_ValtanCanonicalSourceRevision_WhileAdmitted(
			CanonicalAdmission, CanonicalSourceRevision, Diagnostic))
	{
		return PreserveOrCommitReadOnlyProduct(
			"Canonical Balance/Product generation join rejected: " + Diagnostic);
	}
	if (!m_pBalanceTool->Get_ValtanAuthoringView(Staged, Diagnostic))
		return PreserveOrCommitReadOnlyProduct(Diagnostic);
	// Sound 소유 도구의 실패는 해당 lane에 표시하며 무관한 source 편집까지 잠그지 않는다.
	if (m_pAnimationTool)
		(void)m_pAnimationTool->Reload_ValtanCompositionPatternSounds(m_strSoundStatus);
	VALTAN_PATTERN_SHAKE_CUE_DOCUMENT StagedPatternShakes;
	VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT StagedCombatObjectSounds;
	std::string StagedShakeStatus;
	std::string StagedCombatObjectSoundStatus;
	const bool_t bStagedPatternShakesReady =
		CValtanPatternShakeCueDocument::Load_Source(
			StagedPatternShakes, StagedShakeStatus);
	const bool_t bStagedCombatObjectSoundsReady =
		CValtanCombatObjectSoundCueDocument::Load_Source(
			StagedCombatObjectSounds, StagedCombatObjectSoundStatus);
	VALTAN_CANONICAL_READ_DIAGNOSTIC CommitDiagnostic;
	if (!CanonicalAdmission.Validate_StillCurrent(CommitDiagnostic))
	{
		const bool_t bPreserved = 0u != m_CanonicalView.Get_PatternCount();
		m_eAdmission = bPreserved ? VALTAN_VIEW_ADMISSION::STALE_PRESERVED :
			VALTAN_VIEW_ADMISSION::REJECTED;
		m_strStatus = bPreserved ?
			"Canonical generation changed before Workbench commit; previous composition remains read-only: " +
				CommitDiagnostic.strStatus :
			"Canonical generation changed before Workbench commit: " +
				CommitDiagnostic.strStatus;
		return false;
	}

	m_CanonicalView = std::move(Staged);
	m_PlayableInventory = Inventory;
	m_ProductFallbackEncounter.Clear();
	m_bProductFallbackReady = false;
	++m_iCanonicalDisplayGeneration;
	Invalidate_SourceSequenceOwnerIndex();
	m_strPinnedAuthoringSourceRevision = std::move(AuthoringRevision);
	m_strPinnedCanonicalSourceRevision =
		std::move(CanonicalSourceRevision);
	m_bAuthoringDraftDirty = false;
	m_eAdmission = VALTAN_VIEW_ADMISSION::ADMITTED;
	if (!m_bKeepHistoryOnReload) Clear_EditHistory();
	else { m_HistoryValues.reset(); m_FrameHistoryBefore.reset(); }
	m_bConfirmDiscardPatternSoundDraft = false;
	m_strDisplayProvenance =
		"SOURCE / paired gameplay and presentation; authoring revision " +
		m_strPinnedAuthoringSourceRevision + ", canonical source revision " +
		m_strPinnedCanonicalSourceRevision + ".";
	if (m_pAnimationTool)
		m_PatternSoundEvents = m_pAnimationTool->Collect_ValtanCompositionPatternSoundEvents();
	m_bSoundFilterDirty = true;
	m_PatternShakes = std::move(StagedPatternShakes);
	m_bPatternShakesReady = bStagedPatternShakesReady;
	m_strShakeStatus = std::move(StagedShakeStatus);
	m_CombatObjectSounds = std::move(StagedCombatObjectSounds);
	m_bCombatObjectSoundsReady = bStagedCombatObjectSoundsReady;
	m_strCombatObjectSoundStatus =
		std::move(StagedCombatObjectSoundStatus);
	// source 저장·재열기는 현재 Server generation에 그 내용을 적용하는 동작이 아니다.
	m_strStatus = "Canonical composition admitted: " +
		std::to_string(m_CanonicalView.Get_PatternCount()) + " patterns / " +
		std::to_string(m_CanonicalView.Get_StageCount()) + " stages / " +
		std::to_string(Inventory.Get_PatternCount()) +
		" Complete Play patterns. " + Diagnostic + " " + m_strSoundStatus;
	/* stable cue ID가 같아도 원본 필드는 달라졌을 수 있다.
	   이전 세대의 편집용 값 복사본이 새로 승인된 내용을 덮지 않도록 초기화한다. */
	Reset_EffectCueEditor();
	/* Effect V2 clip binding은 Resource 탭만의 부가 정보가 아니라 주 Sequencer의 표시 입력이다.
	   canonical graph와 함께 BOSS_VALTAN catalog를 한 번 준비해 첫 timeline 프레임에도 group box가 나타나게 한다.
	   여기의 Reload_SemanticValtanEffects는 저작 Valtan catalog를 읽으며 저장소 전체 탐색이나 매 프레임 로드는 하지 않는다. */
	// 승인된 V2 소유 바이트는 이미 Save 완료 결과와 일치한다. resource 본문은 바뀌지 않았으므로
	// 그 snapshot은 유지하고 표시용 metadata만 다시 만든다.
	Reload_SemanticValtanEffects(!bReuseCommittedSource);
	Invalidate_TimelineCache();
	Normalize_Selection();
	return true;
}

bool_t Client::CValtanActionWorkbench::Stage_ProductFallback(
	const CValtanCanonicalProductReadAdmission& Admission,
	const std::string& strStrictFailure)
{
	CEncounterPatternReference Staged;
	std::string ProductStatus;
	if (!Staged.Load(
			CProjectDataRoot::Resolve(
				L"Encounters/Valtan/ValtanEncounter.json"),
			ProductStatus))
	{
		m_strStatus = m_bProductFallbackReady ?
			"READ-ONLY PRODUCT FALLBACK STALE_PRESERVED; strict graph and fallback refresh failed. Strict failure: " +
				strStrictFailure + " | Product failure: " + ProductStatus :
			"Canonical graph admission and generated Product fallback both failed: " +
				strStrictFailure + " | " + ProductStatus;
		return false;
	}
	std::string CurrentStatus;
	if (!Admission.Validate_StillCurrent(CurrentStatus))
	{
		m_strStatus = m_bProductFallbackReady ?
			"READ-ONLY PRODUCT FALLBACK STALE_PRESERVED; Product generation changed before fallback commit: " + CurrentStatus :
			"Generated Product fallback generation changed before commit: " +
				CurrentStatus;
		return false;
	}
	m_ProductFallbackEncounter = std::move(Staged);
	m_bProductFallbackReady = true;
	const auto& Patterns = m_ProductFallbackEncounter.Get_Patterns();
	const ENCOUNTER_PATTERN_REFERENCE* pSelected =
		m_ProductFallbackEncounter.Find_Pattern(m_strSelectedPatternId);
	if (nullptr == pSelected && !Patterns.empty())
		pSelected = &Patterns.front();
	m_strSelectedPatternId = nullptr == pSelected ?
		std::string{} : pSelected->patternId;
	m_strSelectedStageId = nullptr == pSelected || pSelected->stages.empty() ?
		std::string{} : pSelected->stages.front().stageId;
	m_strStatus =
		"READ-ONLY PRODUCT FALLBACK: " + std::to_string(Patterns.size()) +
		" generated Product patterns remain visible in Composition Actions. Save, preview, authoring, and Server playback stay blocked. Strict failure: " +
		strStrictFailure;
	return true;
}

void Client::CValtanActionWorkbench::Select_Pattern(
	const VALTAN_PATTERN_VIEW& Pattern)
{
	if (m_strSelectedPatternId != Pattern.strPatternId)
		m_ePreviewPath = InitialPreviewPath(Pattern);
	m_strSelectedPatternId = Pattern.strPatternId;
	m_strSelectedStageId = Pattern.Stages.empty() ?
		std::string{} : Pattern.Stages.front().strStageId;
	m_strSelectedStableId = Pattern.strPatternId;
	m_eDetailOwner = DETAIL_OWNER::PATTERN;
	m_bDetailFocusRequested = false;
	m_iPlayheadMs = 0u;
	m_strSoundAddClipOccurrenceId.clear();
	m_iSoundAddStartMs = 0u;
	m_eSoundAddRepeatPolicy = VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
	m_strResourceTargetPatternId = Pattern.strPatternId;
	m_strResourceTargetStageId = m_strSelectedStageId;
	m_strResourceTargetClipOccurrenceId.clear();
	m_iEffectV2AddStartMs = 0u;
	m_bEffectAddUsesStageClock = true;
	Reset_EffectCueEditor();
	/* Pattern은 같은 프레임의 다음 창도 사용하는 불변 effective snapshot을 가리킬 수 있다.
	   지금 저장소를 지우지 않는다. 선택 stable ID가 달라지면 다음 프레임에 cache key가 불일치해 안전한 시점에 교체된다. */
	Invalidate_TimelineCache();
}

void Client::CValtanActionWorkbench::Request_PatternSelection(
	const VALTAN_PATTERN_VIEW& Pattern,
	const std::string& strStageId,
	const DETAIL_OWNER eDetailOwner,
	const std::string& strStableId,
	const bool_t bOpenSequencer)
{
	if (nullptr != m_pBalanceTool &&
		(m_pBalanceTool->Is_ValtanSaveJobBlockingAuthoring() ||
		 m_pBalanceTool->Is_ServerRuntimeSetPublishRunning()))
	{
		m_strStatus =
			"Pattern selection is blocked until the exact Save/publish receipt is terminal; the current selection was preserved.";
		return;
	}
	if (Pattern.strPatternId.empty() ||
		(Pattern.strPatternId == m_strSelectedPatternId &&
		 strStageId.empty() && !bOpenSequencer))
	{
		return;
	}
	m_PendingPatternSelection = PENDING_PATTERN_SELECTION{
		Pattern.strPatternId, strStageId, eDetailOwner, strStableId, bOpenSequencer };
	m_ePendingPatternSelectionDecision =
		PENDING_PATTERN_SELECTION_DECISION::NONE;
	std::string DraftStatus;
	if ((!bOpenSequencer || Pattern.strPatternId != m_strSelectedPatternId) &&
		Has_UnsavedCompositionDrafts(DraftStatus))
	{
		m_bPendingPatternSelectionModalRequested = true;
		m_strStatus =
			"Pattern switch is waiting for Save, Discard, or Cancel. Current Pattern " +
			m_strSelectedPatternId + ", requested Pattern " +
			Pattern.strPatternId + ". " + DraftStatus;
		return;
	}
	/* 미저장 변경이 없는 선택 이동도 모든 창이 현재 불변 Pattern/Stage 포인터의 사용을 마친 뒤 처리한다. */
	m_bPendingPatternSelectionModalRequested = false;
	m_ePendingPatternSelectionDecision =
		PENDING_PATTERN_SELECTION_DECISION::SELECT;
}

void Client::CValtanActionWorkbench::Select_Stage(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const DETAIL_OWNER eOwner,
	const std::string& strStableId, const bool_t bPreserveTimelineSelection)
{
	if (!bPreserveTimelineSelection) m_TimelineSelection.clear();
	if (m_strSelectedPatternId != Pattern.strPatternId)
		m_ePreviewPath = InitialPreviewPath(Pattern);
	m_strSelectedPatternId = Pattern.strPatternId;
	m_strSelectedStageId = Stage.strStageId;
	m_eDetailOwner = eOwner;
	m_strSelectedStableId = NormalizeBranchStableId(
		Stage.strStageId,
		strStableId.empty() ? Stage.strStageId : strStableId);
	m_bDetailFocusRequested = true;
	m_strSoundAddClipOccurrenceId.clear();
	m_iSoundAddStartMs = 0u;
	m_eSoundAddRepeatPolicy = VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
	Reset_EffectCueEditor();
}

bool_t Client::CValtanActionWorkbench::Reload_AnimationSequences()
{
	m_bAnimationSequenceLoadAttempted = true;
	m_FilteredAnimationSequenceIndices.clear();
	m_bAnimationSequenceFilterDirty = true;
	if (nullptr == m_pAnimationTool)
	{
		m_AnimationSequences.clear();
		m_strAnimationSequenceStatus =
			"Animation preview owner is unavailable.";
		return false;
	}
	std::vector<CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW> Staged;
	if (!m_pAnimationTool->Get_ValtanCompositionSequences(
			Staged, m_strAnimationSequenceStatus))
	{
		m_AnimationSequences.clear();
		return false;
	}
	m_AnimationSequences = std::move(Staged);
	if (m_AnimationSequences.empty())
	{
		m_strSelectedSequenceStableId.clear();
		m_iSelectedSequenceSkillId = -1;
		m_iSelectedSequenceIndex = -1;
		return false;
	}
	const auto Selected = std::find_if(
		m_AnimationSequences.begin(), m_AnimationSequences.end(),
		[this](const CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW& Sequence)
		{
			return !m_strSelectedSequenceStableId.empty() &&
				Sequence.strStableId == m_strSelectedSequenceStableId;
		});
	if (Selected == m_AnimationSequences.end())
	{
		m_strSelectedSequenceStableId =
			m_AnimationSequences.front().strStableId;
		m_iSelectedSequenceSkillId = m_AnimationSequences.front().iSkillId;
		m_iSelectedSequenceIndex = m_AnimationSequences.front().iSequenceIndex;
	}
	return true;
}

bool_t Client::CValtanActionWorkbench::Append_SelectedSequenceToPattern(
	const VALTAN_PATTERN_VIEW& Pattern)
{
	if (!m_pBalanceTool || !m_pAnimationTool || !Can_MutateValtanView(m_eAdmission) ||
		m_pBalanceTool->Is_ValtanSaveJobBlockingAuthoring() || m_pBalanceTool->Is_ServerRuntimeSetPublishRunning() ||
		m_pAnimationTool->Is_ValtanCompositionPatternTransactionActive())
	{ m_strStatus = "Append to Pattern End requires an admitted, idle authoring draft."; return false; }
	VALTAN_PATTERN_VIEW current;
	if (!m_pBalanceTool->Get_ValtanPatternDraft(Pattern.strPatternId, current, m_strStatus) || current.Stages.empty())
		return false;
	const auto selected = std::find_if(m_AnimationSequences.begin(), m_AnimationSequences.end(),
		[this](const auto& row) { return row.strStableId == m_strSelectedSequenceStableId; });
	if (selected == m_AnimationSequences.end() || selected->Clips.empty())
	{ m_strStatus = "Select one non-empty Animation Sequence before appending."; return false; }
	if (!current.bManualServerAudition)
	{
		// canonical gameplay가 Stage graph를 소유한다. 최종 presentation playlist의 정확한 clip 편집만 바꾸며
		// World/Motion/Counter의 소유 도구는 옮기지 않는다.
		const auto target = current.Stages.back();
		const auto savedStage = m_strSelectedStageId, savedStable = m_strSelectedStableId;
		const auto savedOwner = m_eDetailOwner;
		const auto savedSelection = m_TimelineSelection;
		VALTAN_PATTERN_VIEW candidate;
		std::string status;
		const bool applied = m_pBalanceTool->Apply_ValtanCompositionDraftTransaction([&](std::string& diagnostic) {
			if (!Apply_SelectedSequenceToStage(current, target, true)) { diagnostic = m_strStatus; return false; }
			diagnostic = m_strStatus;
			return m_pBalanceTool->Get_ValtanPatternDraft(current.strPatternId, candidate, diagnostic);
		}, status);
		if (!applied)
		{
			m_strSelectedStageId = savedStage; m_strSelectedStableId = savedStable; m_eDetailOwner = savedOwner;
			m_TimelineSelection = savedSelection;
			m_strStatus = "Append to Pattern End rejected; draft and selection preserved. " + status;
			return false;
		}
		m_strSelectedPatternId = current.strPatternId; m_strSelectedStageId = target.strStageId;
		m_TimelineSelection.clear();
		for (const auto& clip : candidate.Stages.back().ClipOccurrences)
			if (std::none_of(target.ClipOccurrences.begin(), target.ClipOccurrences.end(),
				[&](const auto& old) { return old.strClipOccurrenceId == clip.strClipOccurrenceId; }))
				m_TimelineSelection.push_back({current.strPatternId, target.strStageId, clip.strClipOccurrenceId, DETAIL_OWNER::ANIMATION});
		m_bAuthoringDraftDirty = m_pBalanceTool->Is_ValtanDraftDirty();
		m_strEffectEditIdentity.clear();
		Invalidate_EffectivePatternCache(); Invalidate_TimelineCache();
		m_strStatus = "Appended the Sequence after the final Animation in " + target.strStageId +
			". Existing gameplay Stages and their resources are preserved. Save when ready. " + status;
		Refresh_PatternLocalPreviewAfterMutation(&candidate, m_strStatus);
		return true;
	}
	if (!m_pBalanceTool->Can_Edit_ValtanManualStageTopology(current.strPatternId, m_strStatus)) return false;
	uint64_t hash = 1469598103934665603ull;
	for (const unsigned char ch : current.strPatternId) { hash ^= ch; hash *= 1099511628211ull; }
	std::string stageId, actionId;
	for (uint32_t ordinal = 1u; ordinal <= 9999u; ++ordinal)
	{
		const auto id = "SEQUENCE_APPEND_" + std::to_string(ordinal);
		if (std::none_of(current.Stages.begin(), current.Stages.end(), [&](const auto& row) { return row.strStageId == id; }))
		{
			stageId = id;
			actionId = "valtan.sequence.append." + std::to_string(hash) + ".stage." + std::to_string(ordinal);
			break;
		}
	}
	if (stageId.empty()) { m_strStatus = "No free end Stage identity remains; draft preserved."; return false; }
	const auto savedStage = m_strSelectedStageId, savedStable = m_strSelectedStableId;
	const auto savedOwner = m_eDetailOwner;
	const auto savedSelection = m_TimelineSelection;
	VALTAN_PATTERN_VIEW candidate;
	std::string status;
	const bool applied = m_pBalanceTool->Apply_ValtanCompositionDraftTransaction([&](std::string& diagnostic) {
		if (!m_pBalanceTool->Insert_ValtanManualStageAfter(current.strPatternId, current.Stages.back().strStageId,
			stageId, actionId, "ACTIVE", 1u, diagnostic) ||
			!m_pBalanceTool->Get_ValtanPatternDraft(current.strPatternId, candidate, diagnostic)) return false;
		const auto added = std::find_if(candidate.Stages.begin(), candidate.Stages.end(), [&](const auto& row) { return row.strStageId == stageId; });
		if (added == candidate.Stages.end()) { diagnostic = "New end Stage did not resolve."; return false; }
		if (!Apply_SelectedSequenceToStage(candidate, *added, true)) { diagnostic = m_strStatus; return false; }
		diagnostic = m_strStatus;
		return m_pBalanceTool->Get_ValtanPatternDraft(current.strPatternId, candidate, diagnostic);
	}, status);
	if (!applied)
	{
		m_strSelectedStageId = savedStage; m_strSelectedStableId = savedStable; m_eDetailOwner = savedOwner;
		m_TimelineSelection = savedSelection;
		m_strStatus = "Append to Pattern End rejected; draft and selection preserved. " + status;
		return false;
	}
	m_strSelectedPatternId = current.strPatternId;
	m_strSelectedStageId = stageId;
	m_TimelineSelection.clear();
	m_TimelineSelection.push_back({current.strPatternId, stageId, stageId, DETAIL_OWNER::GAMEPLAY_STAGE});
	const auto added = std::find_if(candidate.Stages.begin(), candidate.Stages.end(), [&](const auto& row) { return row.strStageId == stageId; });
	for (const auto& clip : added->ClipOccurrences)
		m_TimelineSelection.push_back({current.strPatternId, stageId, clip.strClipOccurrenceId, DETAIL_OWNER::ANIMATION});
	m_bAuthoringDraftDirty = m_pBalanceTool->Is_ValtanDraftDirty();
	m_BossPatternOutcomeOverrides.clear(); ++m_iBossPatternRouteGeneration;
	m_strEffectEditIdentity.clear();
	Invalidate_EffectivePatternCache(); Invalidate_TimelineCache();
	m_strStatus = "Appended the Sequence in a new Stage at the Pattern end. Earlier/Later moves the selected Stage and clips together. Save when ready. " + status;
	Refresh_PatternLocalPreviewAfterMutation(&candidate, m_strStatus);
	return true;
}

bool_t Client::CValtanActionWorkbench::Apply_SelectedSequenceToStage(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const bool_t bAppend)
{
	if (VALTAN_VIEW_ADMISSION::ADMITTED != m_eAdmission ||
		nullptr == m_pBalanceTool || nullptr == m_pAnimationTool)
	{
		m_strStatus =
			"Sequence slot edit blocked until canonical admission and the typed authoring/native model owners are available.";
		return false;
	}
	if ("WAIT" == Stage.strSequenceRole)
	{
		m_strStatus =
			"Sequence slot edit rejected: WAIT is an explicit blank Stage and must remain Animation NONE. Insert or select an ACTIVE Stage before assigning a Sequence.";
		return false;
	}
	const auto Selected = std::find_if(
		m_AnimationSequences.begin(), m_AnimationSequences.end(),
		[this](const CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW& Sequence)
		{
			return Sequence.strStableId == m_strSelectedSequenceStableId;
		});
	if (Selected == m_AnimationSequences.end() || Selected->Clips.empty())
	{
		m_strStatus = "Select one non-empty extracted Animation Sequence first.";
		return false;
	}
	CBalanceTool::PATTERN_STAGE_EDIT Draft;
	std::string Status;
	if (!m_pBalanceTool->Get_ValtanStageDraft(
			Pattern.strPatternId, Stage.strStageId, Draft, Status) ||
		!Draft.animationEditable)
	{
		m_strStatus = Status.empty() ?
			"This Stage does not admit Animation slot edits." : std::move(Status);
		return false;
	}
	std::vector<uint32_t> NativeDurationsMs;
	NativeDurationsMs.reserve(Selected->Clips.size());
	std::vector<uint32_t> RequestedCutsMs;
	RequestedCutsMs.reserve(Selected->Clips.size());
	std::vector<std::string> ReplacementRoles;
	ReplacementRoles.reserve(Selected->Clips.size());
	std::size_t iNativeBoundedCutCount = 0u;
	std::string NativeBoundedCutExample;
	for (std::size_t iClip = 0u; iClip < Selected->Clips.size(); ++iClip)
	{
		const CAnimation_Tool::COMPOSITION_SEQUENCE_CLIP_VIEW& Clip =
			Selected->Clips[iClip];
		if (0u == Clip.iDurationMs)
		{
			m_strStatus = "Sequence slot edit rejected: source duration is unresolved for " +
				Clip.strClipName + ". Open Animation Clips and review its native timing.";
			return false;
		}
		uint32_t iNativeDurationMs = 0u;
		if (!m_pAnimationTool->Resolve_ValtanCompositionNativeClipDurationMs(
				Clip.strClipName, iNativeDurationMs, Status))
		{
			m_strStatus =
				"Sequence slot edit rejected before the typed draft changed: " + Status;
			return false;
		}
		const std::string Role = ClipReplacementRole(Clip.strClipName);
		uint32_t iPlayableCutMs = Clip.iDurationMs;
		/* 원본 cut은 package 시각, CModel은 변환된 모델의 시각을 사용한다. 반복이 아닌 source entry마다 유한한 native 구간 하나를 가져온다.
		   package cut이 더 길다는 이유로 Sequence 전체를 거절하거나 일회 재생 횟수를 임의로 늘리지 않는다.
		   명시적 _loop clip은 기존의 유한 반복 전개를 유지한다. */
		if (iPlayableCutMs > iNativeDurationMs && "loop" != Role)
		{
			iPlayableCutMs = iNativeDurationMs;
			++iNativeBoundedCutCount;
			if (NativeBoundedCutExample.empty())
			{
				NativeBoundedCutExample = Clip.strClipName + " " +
					std::to_string(Clip.iDurationMs) + " -> " +
					std::to_string(iPlayableCutMs) + " ms";
			}
		}
		NativeDurationsMs.push_back(iNativeDurationMs);
		RequestedCutsMs.push_back(iPlayableCutMs);
		ReplacementRoles.push_back(Role);
	}

	std::vector<uint32_t> FittedCutsMs = RequestedCutsMs;
	std::string FitStatus;
	const uint64_t iRequestedSequenceMs = std::accumulate(
		RequestedCutsMs.begin(), RequestedCutsMs.end(), uint64_t{ 0u });
	/* 추가 가능 여부는 마지막 slot이 결정한다. Stage 끝까지 반복하거나 native 길이를 사용하는 마지막 slot은
	   그 뒤에 붙일 고정 종료 시각이 없다. 앞쪽의 EXACT slot은 끝에 Sequence를 추가하는 것을 막지 않는다. */
	if (bAppend && !Draft.animationSlots.empty() &&
		(0u == Draft.animationSlots.back().playMs ||
		 Draft.animationSlots.back().repeatUntilStageEnd))
	{
		m_strStatus = "Append rejected: the last slot loops to the Stage end or uses its native duration, so nothing can follow it. Replace that slot, or turn off Loop to Stage End and give it an exact Play Duration, then append.";
		return false;
	}
	if (!bAppend && iRequestedSequenceMs > Draft.durationMs)
	{
		const bool_t bDeterministicHoldChain =
			IsRoleAwareHoldReplacementChain(ReplacementRoles);
		if ("HOLD" != Selected->strMode || !bDeterministicHoldChain ||
			!FitCompositionSequenceCutsToStage(
				RequestedCutsMs, ReplacementRoles, Draft.durationMs,
				FittedCutsMs, FitStatus))
		{
			m_strStatus = FitStatus.empty() ?
				"Replace rejected: the selected Sequence requires " +
					std::to_string(iRequestedSequenceMs) +
					" ms, but the selected Server Stage is " +
					std::to_string(Draft.durationMs) +
					" ms and has no deterministic role-aware HOLD-chain fit policy." :
				std::move(FitStatus);
			return false;
		}
	}

	struct EXPANDED_SEQUENCE_CLIP final
	{
		std::string strClip;
		uint32_t iPlayMs = 0u;
	};
	std::vector<EXPANDED_SEQUENCE_CLIP> ExpandedClips;
	for (std::size_t iClip = 0u; iClip < Selected->Clips.size(); ++iClip)
	{
		uint32_t iRemainingMs = FittedCutsMs[iClip];
		if (iRemainingMs > NativeDurationsMs[iClip] &&
			"loop" != ClipReplacementRole(
				Selected->Clips[iClip].strClipName))
		{
			m_strStatus =
				"Sequence slot edit rejected: fitted non-loop cut " +
				Selected->Clips[iClip].strClipName + " requests " +
				std::to_string(iRemainingMs) + " ms but its native window is " +
				std::to_string(NativeDurationsMs[iClip]) +
				" ms; start/end/one-shot clips are never repeated by inference.";
			return false;
		}
		while (iRemainingMs > NativeDurationsMs[iClip])
		{
			ExpandedClips.push_back({
				Selected->Clips[iClip].strClipName,
				NativeDurationsMs[iClip] });
			iRemainingMs -= NativeDurationsMs[iClip];
			if (ExpandedClips.size() > MAX_BOSS_PATTERN_ANIMATION_CLIPS)
			{
				m_strStatus =
					"Sequence slot edit rejected: native-loop materialization exceeds 256 exact occurrences.";
				return false;
			}
		}
		if (0u != iRemainingMs)
		{
			ExpandedClips.push_back({
				Selected->Clips[iClip].strClipName, iRemainingMs });
		}
	}
	if (ExpandedClips.size() + (bAppend ? Draft.animationSlots.size() : 0u) >
		MAX_BOSS_PATTERN_ANIMATION_CLIPS)
	{
		m_strStatus = "Sequence slot edit rejected: a Stage supports at most 256 Animation slots. Add a Stage for the remaining clips.";
		return false;
	}
	if (ExpandedClips.empty())
	{
		m_strStatus = "Sequence slot edit rejected: fitted Sequence is empty.";
		return false;
	}

	std::vector<CBalanceTool::ANIMATION_SLOT_EDIT> Slots =
		bAppend ? Draft.animationSlots :
		std::vector<CBalanceTool::ANIMATION_SLOT_EDIT>{};
	const std::size_t iExistingCount = Draft.animationSlots.size();
	/* Replace는 occurrence ID를 논리적인 timeline slot의 안정적인 식별자로 취급한다.
	   정확히 같은 clip을 우선 보존하고 다음으로 start/loop/end 역할이 유일할 때 연결한다.
	   역할이 반복되어 모호하면 새 ID를 부여해 Effect/Sound/Shake가 임의의 반복을 따라가지 않고 검증에서 거절되게 한다.
	   내용이 바뀐 유일 역할은 PROJECT_AUTHORED로 표시하며 draft를 바꾸기 전에 연결된 원본 구간을 모두 다시 검증한다. */
	std::vector<CBalanceTool::ANIMATION_SLOT_EDIT> ReservedSlots =
		Draft.animationSlots;
	std::vector<bool_t> ReusedExistingSlots(iExistingCount, false);
	std::vector<std::size_t> ReusableSlots(
		ExpandedClips.size(), iExistingCount);
	if (!bAppend)
	{
		for (std::size_t iClip = 0u; iClip < ExpandedClips.size(); ++iClip)
		{
			for (std::size_t iExisting = 0u;
				iExisting < iExistingCount; ++iExisting)
			{
				if (!ReusedExistingSlots[iExisting] &&
					Draft.animationSlots[iExisting].clip ==
						ExpandedClips[iClip].strClip)
				{
					ReusableSlots[iClip] = iExisting;
					ReusedExistingSlots[iExisting] = true;
					break;
				}
			}
		}
		for (std::size_t iClip = 0u; iClip < ExpandedClips.size(); ++iClip)
		{
			if (ReusableSlots[iClip] < iExistingCount)
				continue;
			const std::string Role =
				ClipReplacementRole(ExpandedClips[iClip].strClip);
			if (Role.empty())
				continue;
			const std::size_t iUnassignedNewRoleCount =
				static_cast<std::size_t>(std::count_if(
					ExpandedClips.begin(), ExpandedClips.end(),
					[&ExpandedClips, &ReusableSlots, iExistingCount, &Role](
						const EXPANDED_SEQUENCE_CLIP& Candidate)
					{
						const std::size_t iCandidate = static_cast<std::size_t>(
							&Candidate - ExpandedClips.data());
						return ReusableSlots[iCandidate] == iExistingCount &&
							Role == ClipReplacementRole(Candidate.strClip);
					}));
			std::size_t iUniqueExisting = iExistingCount;
			std::size_t iUnassignedExistingRoleCount = 0u;
			for (std::size_t iExisting = 0u;
				iExisting < iExistingCount; ++iExisting)
			{
				if (!ReusedExistingSlots[iExisting] &&
					Role == ClipReplacementRole(
						Draft.animationSlots[iExisting].clip))
				{
					iUniqueExisting = iExisting;
					++iUnassignedExistingRoleCount;
				}
			}
			if (1u == iUnassignedNewRoleCount &&
				1u == iUnassignedExistingRoleCount)
			{
				ReusableSlots[iClip] = iUniqueExisting;
				ReusedExistingSlots[iUniqueExisting] = true;
			}
		}
	}
	uint64_t iDurationMs = 0u;
	for (const CBalanceTool::ANIMATION_SLOT_EDIT& Slot : Slots)
		iDurationMs += static_cast<uint64_t>(std::llround(
			static_cast<double>(Slot.playMs) / Slot.playRate));
	for (std::size_t iClip = 0u; iClip < ExpandedClips.size(); ++iClip)
	{
		CBalanceTool::ANIMATION_SLOT_EDIT Slot;
		bool_t bReusedIdentity = false;
		const std::size_t iReusableSlot = ReusableSlots[iClip];
		if (iReusableSlot < iExistingCount)
		{
			Slot.clipOccurrenceId =
				Draft.animationSlots[iReusableSlot].clipOccurrenceId;
			Slot.mappingBasis =
				Draft.animationSlots[iReusableSlot].clip ==
					ExpandedClips[iClip].strClip ?
					Draft.animationSlots[iReusableSlot].mappingBasis :
					"PROJECT_AUTHORED";
			bReusedIdentity = true;
		}
		else
		{
			Slot.clipOccurrenceId = BuildNextCompositionSlotId(
				Pattern.strPatternId, Stage.strStageId, ReservedSlots);
			if (Slot.clipOccurrenceId.empty())
			{
				m_strStatus =
					"Sequence slot edit rejected: no free stable occurrence ID is available.";
				return false;
			}
			Slot.mappingBasis = "PROJECT_AUTHORED";
		}
		Slot.clip = ExpandedClips[iClip].strClip;
		Slot.sourceStartMs = 0u;
		Slot.playRate = 1.0;
		Slot.playMs = ExpandedClips[iClip].iPlayMs;
		Slot.repeatUntilStageEnd = false;
		if (!bReusedIdentity)
			ReservedSlots.push_back(Slot);
		Slots.push_back(std::move(Slot));
		iDurationMs += ExpandedClips[iClip].iPlayMs;
	}
	if (iDurationMs < 1u || iDurationMs > 600000u)
	{
		m_strStatus = "Sequence slot edit rejected: combined wall clock is outside 1..600000 ms.";
		return false;
	}
	Draft.animationSlots = std::move(Slots);
	if (bAppend)
		Draft.durationMs = (std::max)(Draft.durationMs, static_cast<uint32_t>(iDurationMs));
	Draft.animationEndPolicy = iDurationMs == Draft.durationMs ?
		"EXACT" : "HOLD_LAST_POSE";
	Draft.animationRepeatCount = 1u;
	if (!SetValtanStageDraftWithSoundDependencyAdmission(
			m_pAnimationTool, m_pBalanceTool,
			m_bPatternShakesReady ? &m_PatternShakes : nullptr,
			Pattern, Stage, Draft, Status,
			static_cast<uint32_t>(Selected->iSkillId),
			static_cast<uint32_t>(Selected->iSequenceIndex)))
	{
		m_strStatus = std::move(Status);
		return false;
	}
	if (!Draft.animationSlots.empty())
	{
		const std::size_t iSelectedSlot = bAppend ?
			(std::min)(iExistingCount, Draft.animationSlots.size() - 1u) : 0u;
		m_strSelectedStableId =
			Draft.animationSlots[iSelectedSlot].clipOccurrenceId;
		m_eDetailOwner = DETAIL_OWNER::ANIMATION;
	}
	m_strStatus = std::string(bAppend ? "Appended " : "Replaced with ") +
		std::to_string(ExpandedClips.size()) +
		" exact Sequence slots in the shared authoring draft; Server Stage clock " +
		std::to_string(Draft.durationMs) + " ms was " +
		(bAppend ? "extended" : "preserved") +
		(FitStatus.empty() ? ". " : ". " + FitStatus + " ");
	if (0u != iNativeBoundedCutCount)
	{
		m_strStatus += "Imported " + std::to_string(iNativeBoundedCutCount) +
			" non-loop source cut(s) within their native playback window (" +
			NativeBoundedCutExample + "). ";
	}
	m_strStatus += "Press Save.";
	Invalidate_TimelineCache();
	return true;
}

bool_t Client::CValtanActionWorkbench::Remove_AnimationOccurrence(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const std::string& strClipOccurrenceId,
	std::string& strOutStatus)
{
	if (nullptr == m_pAnimationTool || nullptr == m_pBalanceTool ||
		strClipOccurrenceId.empty())
	{
		strOutStatus =
			"Delete requires one selected Animation box and both typed owners.";
		return false;
	}
	// 값 복사본을 사용해 중첩 소유 도구 transaction과 cache 갱신 뒤에도 입력 값을 유지한다.
	const VALTAN_PATTERN_VIEW original = Pattern;
	const VALTAN_STAGE_VIEW source = Stage;
	const std::string clipId = strClipOccurrenceId;
	CBalanceTool::PATTERN_STAGE_EDIT Draft;
	if (!m_pBalanceTool->Get_ValtanStageDraft(
			Pattern.strPatternId, Stage.strStageId, Draft, strOutStatus))
	{
		return false;
	}
	const auto Found = std::find_if(
		Draft.animationSlots.begin(), Draft.animationSlots.end(),
		[&strClipOccurrenceId](
			const CBalanceTool::ANIMATION_SLOT_EDIT& Slot)
		{
			return Slot.clipOccurrenceId == strClipOccurrenceId;
		});
	if (Draft.animationSlots.end() == Found)
	{
		strOutStatus =
			"Delete rejected: the selected Animation box is not in the current Stage draft.";
		return false;
	}
	const auto effectV2Snapshot = CEffectV2Catalog::Get().Get_Snapshot();
	if (!effectV2Snapshot || !effectV2Snapshot->Is_Ready() ||
		!effectV2Snapshot->Can_MutateBossValtanBindings())
	{
		strOutStatus = "Delete requires the complete Effect V2 binding view; all drafts were preserved.";
		return false;
	}
	std::vector<EFFECT_V2_STAGE_BINDING_KEY> effectRows;
	for (const auto& binding : effectV2Snapshot->Get_BossValtanBindings())
		if (binding.strPatternId == original.strPatternId &&
			binding.strStageId == source.strStageId &&
			binding.eClockBasis == EFFECT_V2_CLOCK_BASIS::CLIP_OCCURRENCE &&
			binding.strClipOccurrenceId == clipId)
		{
			if (binding.strActionId != source.strActionId)
			{
				strOutStatus = "Animation Delete found a stale Effect owner; all drafts were preserved.";
				return false;
			}
			effectRows.push_back(EFFECT_V2_STAGE_BINDING_KEY::From_Binding(binding));
		}
	Draft.animationSlots.erase(Found);
	if (Draft.animationSlots.empty())
	{
		Draft.animationEndPolicy = "NONE";
		Draft.animationRepeatCount = 0u;
	}
	else
	{
		uint32_t iWallMs = 0u;
		if (ComputeExactAnimationWallMs(Draft, iWallMs))
		{
			if (iWallMs > Draft.durationMs)
			{
				strOutStatus =
					"Delete rejected: remaining Animation boxes exceed the preserved Server Stage clock.";
				return false;
			}
			Draft.animationEndPolicy = iWallMs == Draft.durationMs ?
				"EXACT" : "HOLD_LAST_POSE";
		}
		Draft.animationRepeatCount = 1u;
	}
	if (!m_bPatternShakesReady) { strOutStatus = "Animation Delete requires the Camera Shake source; draft preserved. " + m_strShakeStatus; return false; }
	auto candidateShakes = m_PatternShakes;
	if (!CValtanPatternShakeCueDocument::Remove_ClipDraft(candidateShakes, Pattern.strPatternId,
		Stage.strStageId, strClipOccurrenceId, strOutStatus)) return false;
	std::size_t iRemovedSoundRows = 0u;
	std::size_t iRemovedEffectRows = 0u;
	const bool applied = CEffectV2Catalog::Get().Apply_BossValtanBindingDraftTransaction(
		[&](std::string& effectStatus) {
			return m_pAnimationTool->Apply_ValtanCompositionPatternSoundDraftTransaction(
				[&](std::string& soundStatus) {
					return m_pBalanceTool->Apply_ValtanCompositionDraftTransaction(
						[&](std::string& status) {
							// clip이 소유한 cue는 Animation 삭제와 함께 제거한다. 독립 Stage 시각을 사용하는 cue는 그대로 둔다.
							for (const auto& cue : source.ProductCues)
								if (!cue.bUsesStageClock && cue.strClipOccurrenceId == clipId)
								{
									if (!m_pBalanceTool->Remove_ValtanStageEffectCue(
										original.strPatternId, source.strStageId, source.strActionId,
										cue.strBindingId, cue.strOccurrenceId, cue.strEffectAssetId,
										cue.strClipOccurrenceId, status)) return false;
									++iRemovedEffectRows;
								}
							if (!CEffectV2Catalog::Get().Stage_RemoveBossValtanBindings(effectRows, status))
								return false;
							VALTAN_PATTERN_SOUND_CUE_DOCUMENT previousSounds;
							bool_t previousSoundDirty = false;
							uint64_t soundGeneration = 0u;
							if (!m_pAnimationTool->Stage_ValtanCompositionPatternSoundCascadeForAnimationDelete(
								original, source, clipId, previousSounds, previousSoundDirty,
								soundGeneration, iRemovedSoundRows, status)) return false;
							VALTAN_PATTERN_VIEW candidate;
							if (!m_pBalanceTool->Get_ValtanPatternDraft(original.strPatternId, candidate, status))
								return false;
							const auto stage = std::find_if(candidate.Stages.begin(), candidate.Stages.end(),
								[&](const auto& row) { return row.strStageId == source.strStageId; });
							if (stage == candidate.Stages.end() || stage->strActionId != source.strActionId)
							{
								status = "Animation Delete lost its exact Stage owner; all drafts were preserved.";
								return false;
							}
							// 타입별 Stage setter는 읽기 전용으로 결합된 목록도 비교한다.
							Draft.productCues = stage->ProductCues;
							return SetValtanStageDraftWithSoundDependencyAdmission(
								m_pAnimationTool, m_pBalanceTool, &candidateShakes,
								candidate, *stage, Draft, status);
						}, soundStatus);
					}, effectStatus);
			}, strOutStatus);
	if (!applied) return false;
	m_PatternShakes = std::move(candidateShakes);
	m_strSelectedStableId = Draft.animationSlots.empty() ?
		source.strStageId : Draft.animationSlots.front().clipOccurrenceId;
	m_eDetailOwner = Draft.animationSlots.empty() ?
		DETAIL_OWNER::GAMEPLAY_STAGE : DETAIL_OWNER::ANIMATION;
	m_iEffectV2CatalogRevision = CEffectV2Catalog::Get().Get_Revision();
	m_bAuthoringDraftDirty = m_pBalanceTool->Is_ValtanDraftDirty();
	Invalidate_TimelineCache();
	strOutStatus = "Deleted the selected Animation box and its linked resources (" +
		std::to_string(iRemovedEffectRows + effectRows.size()) + " Effect, " +
		std::to_string(iRemovedSoundRows) + " Sound). Independent Stage resources and the Server Stage clock were preserved. Save when ready.";
	return true;
}

bool_t Client::CValtanActionWorkbench::Duplicate_AnimationOccurrence(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const std::string& strClipOccurrenceId,
	std::string& strOutStatus)
{
	if (nullptr == m_pAnimationTool || nullptr == m_pBalanceTool ||
		strClipOccurrenceId.empty())
	{
		strOutStatus =
			"Duplicate requires one selected Animation box and both typed owners.";
		return false;
	}
	CBalanceTool::PATTERN_STAGE_EDIT Draft;
	if (!m_pBalanceTool->Get_ValtanStageDraft(
			Pattern.strPatternId, Stage.strStageId, Draft, strOutStatus))
	{
		return false;
	}
	const auto Found = std::find_if(
		Draft.animationSlots.begin(), Draft.animationSlots.end(),
		[&strClipOccurrenceId](
			const CBalanceTool::ANIMATION_SLOT_EDIT& Slot)
		{
			return Slot.clipOccurrenceId == strClipOccurrenceId;
		});
	if (Draft.animationSlots.end() == Found ||
		Found->repeatUntilStageEnd || 0u == Found->playMs)
	{
		strOutStatus =
			"Duplicate rejected: select one finite Animation box; an open-ended loop must be converted to exact occurrences first.";
		return false;
	}
	CBalanceTool::ANIMATION_SLOT_EDIT Duplicate = *Found;
	Duplicate.clipOccurrenceId = BuildNextCompositionSlotId(
		Pattern.strPatternId, Stage.strStageId, Draft.animationSlots);
	Duplicate.mappingBasis = "PROJECT_AUTHORED";
	if (Duplicate.clipOccurrenceId.empty())
	{
		strOutStatus =
			"Duplicate rejected: no free stable Animation occurrence ID is available.";
		return false;
	}
	const std::size_t iInsertIndex = static_cast<std::size_t>(
		Found - Draft.animationSlots.begin()) + 1u;
	Draft.animationSlots.insert(
		Draft.animationSlots.begin() +
			static_cast<std::ptrdiff_t>(iInsertIndex), Duplicate);
	uint32_t iAnimationWallMs = 0u;
	if (!ComputeExactAnimationWallMs(Draft, iAnimationWallMs))
	{
		strOutStatus =
			"Duplicate rejected: the resulting Animation boxes do not have one finite exact clock.";
		return false;
	}
	if (iAnimationWallMs > Draft.durationMs)
		Draft.durationMs = iAnimationWallMs;
	Draft.animationEndPolicy = iAnimationWallMs == Draft.durationMs ?
		"EXACT" : "HOLD_LAST_POSE";
	const bool_t bOneRepeatedClip = Draft.animationSlots.size() > 1u &&
		std::all_of(
			Draft.animationSlots.begin() + 1u, Draft.animationSlots.end(),
			[&Draft](const CBalanceTool::ANIMATION_SLOT_EDIT& Slot)
			{
				return Slot.clip == Draft.animationSlots.front().clip;
			});
	Draft.animationRepeatCount = bOneRepeatedClip ?
		static_cast<uint32_t>(Draft.animationSlots.size()) : 1u;
	if (!SetValtanStageDraftWithSoundDependencyAdmission(
			m_pAnimationTool, m_pBalanceTool,
			m_bPatternShakesReady ? &m_PatternShakes : nullptr,
			Pattern, Stage, Draft, strOutStatus))
	{
		return false;
	}
	m_eDetailOwner = DETAIL_OWNER::ANIMATION;
	m_strSelectedStableId = Duplicate.clipOccurrenceId;
	Invalidate_TimelineCache();
	strOutStatus =
		"Duplicated the selected Animation box immediately after its source; the Server Stage clock now matches the exact boxes. Use Save when ready.";
	return true;
}

bool_t Client::CValtanActionWorkbench::Reorder_AnimationOccurrence(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const std::string& strClipOccurrenceId,
	const std::size_t iTargetIndex,
	std::string& strOutStatus)
{
	if (nullptr == m_pAnimationTool || nullptr == m_pBalanceTool ||
		strClipOccurrenceId.empty())
	{
		strOutStatus =
			"Animation drag requires one selected box and both typed owners.";
		return false;
	}
	CBalanceTool::PATTERN_STAGE_EDIT Draft;
	if (!m_pBalanceTool->Get_ValtanStageDraft(
			Pattern.strPatternId, Stage.strStageId, Draft, strOutStatus))
	{
		return false;
	}
	const auto Found = std::find_if(
		Draft.animationSlots.begin(), Draft.animationSlots.end(),
		[&strClipOccurrenceId](
			const CBalanceTool::ANIMATION_SLOT_EDIT& Slot)
		{
			return Slot.clipOccurrenceId == strClipOccurrenceId;
		});
	if (Draft.animationSlots.end() == Found)
	{
		strOutStatus =
			"Animation drag rejected: the selected box is not in the current Stage draft.";
		return false;
	}
	const std::size_t iSourceIndex = static_cast<std::size_t>(
		Found - Draft.animationSlots.begin());
	const std::size_t iClampedTarget = (std::min)(
		iTargetIndex, Draft.animationSlots.size() - 1u);
	if (iSourceIndex == iClampedTarget)
	{
		strOutStatus = "Animation box order is unchanged.";
		return true;
	}
	CBalanceTool::ANIMATION_SLOT_EDIT Moving = std::move(*Found);
	Draft.animationSlots.erase(
		Draft.animationSlots.begin() +
			static_cast<std::ptrdiff_t>(iSourceIndex));
	Draft.animationSlots.insert(
		Draft.animationSlots.begin() +
			static_cast<std::ptrdiff_t>((std::min)(
				iClampedTarget, Draft.animationSlots.size())),
		std::move(Moving));
	if (!SetValtanStageDraftWithSoundDependencyAdmission(
			m_pAnimationTool, m_pBalanceTool,
			m_bPatternShakesReady ? &m_PatternShakes : nullptr,
			Pattern, Stage, Draft, strOutStatus))
	{
		return false;
	}
	Invalidate_TimelineCache();
	strOutStatus =
		"Reordered the selected Animation box; linked Effect/Sound/Shake rows kept the same stable occurrence. Use Save when ready.";
	return true;
}

bool_t Client::CValtanActionWorkbench::Transfer_AnimationOccurrence(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& SourceStage,
	const VALTAN_STAGE_VIEW& TargetStage,
	const std::string& strClipOccurrenceId,
	const std::size_t iTargetIndex,
	std::string& strOutStatus)
{
	if (SourceStage.strStageId == TargetStage.strStageId)
	{
		return Reorder_AnimationOccurrence(
			Pattern, SourceStage, strClipOccurrenceId,
			iTargetIndex, strOutStatus);
	}
	if (nullptr == m_pAnimationTool || nullptr == m_pBalanceTool ||
		strClipOccurrenceId.empty())
	{
		strOutStatus =
			"Cross-Stage Animation drag requires one stable box and both typed owners.";
		return false;
	}

	CBalanceTool::PATTERN_STAGE_EDIT SourceDraft;
	CBalanceTool::PATTERN_STAGE_EDIT TargetDraft;
	if (!m_pBalanceTool->Get_ValtanStageDraft(
			Pattern.strPatternId, SourceStage.strStageId,
			SourceDraft, strOutStatus) ||
		!m_pBalanceTool->Get_ValtanStageDraft(
			Pattern.strPatternId, TargetStage.strStageId,
			TargetDraft, strOutStatus))
	{
		return false;
	}
	if (!SourceDraft.animationEditable || !TargetDraft.animationEditable ||
		"WAIT" == TargetStage.strSequenceRole)
	{
		strOutStatus =
			"Cross-Stage Animation drag requires two editable non-WAIT Stage owners.";
		return false;
	}
	const std::size_t iSourceMatchCount = static_cast<std::size_t>(
		std::count_if(
			SourceDraft.animationSlots.begin(), SourceDraft.animationSlots.end(),
			[&strClipOccurrenceId](const CBalanceTool::ANIMATION_SLOT_EDIT& Slot)
			{
				return Slot.clipOccurrenceId == strClipOccurrenceId;
			}));
	const auto Found = std::find_if(
		SourceDraft.animationSlots.begin(), SourceDraft.animationSlots.end(),
		[&strClipOccurrenceId](const CBalanceTool::ANIMATION_SLOT_EDIT& Slot)
		{
			return Slot.clipOccurrenceId == strClipOccurrenceId;
		});
	if (1u != iSourceMatchCount || SourceDraft.animationSlots.end() == Found)
	{
		strOutStatus =
			"Cross-Stage Animation drag lost the exact source clipOccurrenceId: " +
			strClipOccurrenceId + ".";
		return false;
	}
	if (Found->repeatUntilStageEnd || 0u == Found->playMs)
	{
		strOutStatus =
			"Cross-Stage Animation drag requires one finite non-loop box.";
		return false;
	}
	if (1u == SourceDraft.animationSlots.size() &&
		"WAIT" == SourceStage.strSequenceRole)
	{
		strOutStatus =
			"A WAIT Stage retains its typed animation policy; its last slot cannot be moved.";
		return false;
	}

	const bool_t bLinkedEffect = std::any_of(
		SourceStage.ProductCues.begin(), SourceStage.ProductCues.end(),
		[&strClipOccurrenceId](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue)
		{
			return !Cue.bUsesStageClock &&
				Cue.strClipOccurrenceId == strClipOccurrenceId;
		});
	bool_t bPatternSoundDirty = false;
	std::string SoundStatus;
	const VALTAN_PATTERN_SOUND_CUE_DOCUMENT* const pPatternSounds =
		m_pAnimationTool->Get_ValtanCompositionPatternSoundDraft(
			bPatternSoundDirty, SoundStatus);
	if (nullptr == pPatternSounds)
	{
		strOutStatus =
			"Cross-Stage Animation drag could not admit the Pattern Sound dependency owner: " +
			SoundStatus;
		return false;
	}
	const bool_t bLinkedSound = std::any_of(
		pPatternSounds->Cues.begin(), pPatternSounds->Cues.end(),
		[&Pattern, &SourceStage, &strClipOccurrenceId](
			const VALTAN_PATTERN_SOUND_CUE& Cue)
		{
			return Cue.strPatternId == Pattern.strPatternId &&
				Cue.strStageId == SourceStage.strStageId &&
				Cue.strClipOccurrenceId == strClipOccurrenceId;
		});
	if (!m_bPatternShakesReady)
	{
		strOutStatus =
			"Cross-Stage Animation drag is blocked until the Pattern Shake dependency owner is admitted: " +
			m_strShakeStatus;
		return false;
	}
	const bool_t bLinkedShake = std::any_of(
		m_PatternShakes.Cues.begin(), m_PatternShakes.Cues.end(),
		[&Pattern, &SourceStage, &strClipOccurrenceId](
			const VALTAN_PATTERN_SHAKE_CUE& Cue)
		{
			return Cue.strPatternId == Pattern.strPatternId &&
				Cue.strStageId == SourceStage.strStageId &&
				Cue.strClipOccurrenceId == strClipOccurrenceId;
		});
	const auto effectV2Snapshot = CEffectV2Catalog::Get().Get_Snapshot();
	const bool_t bLinkedEffectV2 = !effectV2Snapshot || std::any_of(
		effectV2Snapshot->Get_BossValtanBindings().begin(), effectV2Snapshot->Get_BossValtanBindings().end(),
		[&](const EFFECT_V2_BINDING& Binding) {
			return Binding.strPatternId == Pattern.strPatternId && Binding.strStageId == SourceStage.strStageId &&
				Binding.eClockBasis == EFFECT_V2_CLOCK_BASIS::CLIP_OCCURRENCE && Binding.strClipOccurrenceId == strClipOccurrenceId;
		});
	if (bLinkedEffect || bLinkedSound || bLinkedShake || bLinkedEffectV2)
	{
		strOutStatus =
			"Cross-Stage Animation drag was preserved because the box owns linked " +
			std::string(bLinkedEffect ? "Effect " : "") +
			(bLinkedEffectV2 ? "Effect V2 " : "") +
			(bLinkedSound ? "Sound " : "") +
			(bLinkedShake ? "Shake " : "") +
			"row(s). Move a dependency-free appended box or remove/retarget those exact rows first.";
		return false;
	}

	CBalanceTool::ANIMATION_SLOT_EDIT Moving = *Found;
	SourceDraft.animationSlots.erase(Found);
	TargetDraft.animationSlots.insert(
		TargetDraft.animationSlots.begin() +
			static_cast<std::ptrdiff_t>((std::min)(
				iTargetIndex, TargetDraft.animationSlots.size())),
		std::move(Moving));
	if (!NormalizeAnimationSlotDraftClock(
			SourceDraft, false, strOutStatus) ||
		!NormalizeAnimationSlotDraftClock(
			TargetDraft, true, strOutStatus))
	{
		return false;
	}
	if (!SetValtanStageDraftWithSoundDependencyAdmission(
			m_pAnimationTool, m_pBalanceTool,
			&m_PatternShakes, Pattern, SourceStage,
			SourceDraft, strOutStatus, 0u, 0u, true) ||
		!SetValtanStageDraftWithSoundDependencyAdmission(
			m_pAnimationTool, m_pBalanceTool,
			&m_PatternShakes, Pattern, TargetStage,
			TargetDraft, strOutStatus, 0u, 0u, true))
	{
		return false;
	}
	if (!m_pBalanceTool->Set_ValtanAnimationTransferDrafts(
			Pattern.strPatternId,
			SourceStage.strStageId, SourceDraft,
			TargetStage.strStageId, TargetDraft,
			strOutStatus))
	{
		return false;
	}
	m_strSelectedStageId = TargetStage.strStageId;
	m_strSelectedStableId = strClipOccurrenceId;
	m_eDetailOwner = DETAIL_OWNER::ANIMATION;
	Invalidate_TimelineCache();
	strOutStatus =
		"Moved the selected Animation box from " + SourceStage.strStageId +
		" to " + TargetStage.strStageId +
		" as one rollback-safe two-Stage draft. Use Save when ready.";
	return true;
}

bool_t Client::CValtanActionWorkbench::Validate_EffectV2BindingClock(
	const VALTAN_STAGE_VIEW& Stage,
	const EFFECT_V2_BINDING& Binding,
	std::string& strOutStatus) const
{
	if (EFFECT_V2_CLOCK_BASIS::STAGE == Binding.eClockBasis)
	{
		if (Binding.iStartMs < Stage.iDurationMs)
			return true;
		strOutStatus = "V2 Stage start must precede the Stage end.";
		return false;
	}
	if (EFFECT_V2_CLOCK_BASIS::CLIP_OCCURRENCE != Binding.eClockBasis)
	{
		strOutStatus = "V2 clock basis is invalid.";
		return false;
	}
	const auto Found = std::find_if(Stage.ClipOccurrences.begin(), Stage.ClipOccurrences.end(),
		[&Binding](const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
		{ return Clip.strClipOccurrenceId == Binding.strClipOccurrenceId; });
	if (Stage.ClipOccurrences.end() == Found)
	{
		strOutStatus = "V2 clipOccurrenceId must resolve in the selected Stage.";
		return false;
	}
	if (EFFECT_V2_REPEAT_POLICY::EACH_LOOP == Binding.eRepeatPolicy && !Found->bLoop)
	{
		strOutStatus = "V2 EACH_LOOP requires a looping Animation occurrence.";
		return false;
	}
	uint32_t iSourceDurationMs = Found->iPlayMs;
	if (0u == iSourceDurationMs)
	{
		uint32_t iNativeDurationMs = 0u;
		if (nullptr == m_pAnimationTool ||
			!m_pAnimationTool->Resolve_ValtanCompositionNativeClipDurationMs(
				Found->strClipName, iNativeDurationMs, strOutStatus))
		{
			strOutStatus = "V2 clock edit requires the selected clip's native source duration. " + strOutStatus;
			return false;
		}
		iSourceDurationMs = iNativeDurationMs > Found->iSourceStartMs ?
			iNativeDurationMs - Found->iSourceStartMs : 0u;
	}
	if (Binding.iStartMs < Found->iSourceStartMs ||
		static_cast<uint64_t>(Binding.iStartMs) >=
			static_cast<uint64_t>(Found->iSourceStartMs) + iSourceDurationMs)
	{
		strOutStatus = "V2 start lies outside the selected Animation source window.";
		return false;
	}
	return true;
}

Client::CValtanActionWorkbench::TIMELINE_SELECTION
Client::CValtanActionWorkbench::Timeline_SelectionKey(const TIMELINE_ITEM& Item)
{
	return {Item.strPatternId, Item.strStageId, Item.strStableId, Item.eOwner};
}

bool_t Client::CValtanActionWorkbench::Is_TimelineBoxSelected(const TIMELINE_ITEM& Item) const
{
	const auto key = Timeline_SelectionKey(Item);
	return std::find(m_TimelineSelection.begin(), m_TimelineSelection.end(), key) != m_TimelineSelection.end();
}

void Client::CValtanActionWorkbench::Select_TimelineBox(const TIMELINE_ITEM& Item, const bool_t bToggle)
{
	const auto key = Timeline_SelectionKey(Item);
	if (!bToggle) m_TimelineSelection.clear();
	const auto found = std::find(m_TimelineSelection.begin(), m_TimelineSelection.end(), key);
	if (found == m_TimelineSelection.end()) m_TimelineSelection.push_back(key);
	else if (bToggle) m_TimelineSelection.erase(found);
	if (!m_TimelineSelection.empty())
	{
		const auto& focus = m_TimelineSelection.back();
		m_strSelectedStageId = focus.strStageId;
		m_strSelectedStableId = focus.strStableId;
		m_eDetailOwner = focus.eOwner;
	}
	else
	{
		m_strSelectedStableId.clear();
		m_eDetailOwner = DETAIL_OWNER::GAMEPLAY_STAGE;
	}
}

void Client::CValtanActionWorkbench::Prune_TimelineSelection()
{
	std::erase_if(m_TimelineSelection, [&](const auto& key) {
		return key.strPatternId != m_strSelectedPatternId ||
			std::none_of(m_TimelineItems.begin(), m_TimelineItems.end(), [&](const auto& item) {
				return Timeline_SelectionKey(item) == key;
			});
	});
	if (!m_TimelineSelection.empty() && std::none_of(m_TimelineSelection.begin(), m_TimelineSelection.end(),
		[&](const auto& key) { return key.strStageId == m_strSelectedStageId &&
			key.strStableId == m_strSelectedStableId && key.eOwner == m_eDetailOwner; }))
		m_TimelineSelection.clear();
}

bool_t Client::CValtanActionWorkbench::Apply_TimelineGroupEdit(const VALTAN_PATTERN_VIEW& Pattern,
	const TIMELINE_GROUP_EDIT edit, const int64_t iWallDeltaMs, std::string& status)
{
	if (!m_pBalanceTool || !m_pAnimationTool || !Can_MutateValtanView(m_eAdmission) ||
		m_pBalanceTool->Is_ValtanSaveJobBlockingAuthoring() || m_pBalanceTool->Is_ServerRuntimeSetPublishRunning() ||
		m_pAnimationTool->Is_ValtanCompositionPatternTransactionActive())
	{ status = "Group editing requires an admitted, idle source draft."; return false; }
	if (edit == TIMELINE_GROUP_EDIT::DUPLICATE_BOXES)
		return Execute_CompositionEdit(COMPOSITION_EDIT_COMMAND::DUPLICATE_SELECTION, status);
	const bool hasStage = std::any_of(m_TimelineSelection.begin(), m_TimelineSelection.end(),
		[](const auto& key) { return key.eOwner == DETAIL_OWNER::GAMEPLAY_STAGE && key.strStableId == key.strStageId; });
	if (hasStage)
		return edit == TIMELINE_GROUP_EDIT::DELETE_BOXES ? Delete_SelectedStages(Pattern, status) : Move_SelectedStages(Pattern, iWallDeltaMs, status);
	struct ROW
	{
		TIMELINE_ITEM item;
		std::optional<VALTAN_PRODUCT_EFFECT_CUE_VIEW> effect;
		std::optional<VALTAN_PATTERN_SOUND_CUE> sound;
		std::optional<EFFECT_V2_BINDING> v2;
		std::optional<CBalanceTool::PATTERN_STAGE_EDIT> collider;
		bool detached = false;
	};
	std::vector<ROW> rows;
	std::set<std::string> animationIds;
	bool soundDirty = false;
	const auto* sounds = m_pAnimationTool->Get_ValtanCompositionPatternSoundDraft(soundDirty, status);
	const auto effects = CEffectV2Catalog::Get().Get_Snapshot();
	if (!sounds || !effects || !effects->Can_MutateBossValtanBindings())
	{ status = "Group editing needs the admitted Sound and Effect binding drafts."; return false; }
	for (const auto& key : m_TimelineSelection)
	{
		const auto item = std::find_if(m_TimelineItems.begin(), m_TimelineItems.end(),
			[&](const auto& value) { return Timeline_SelectionKey(value) == key; });
		const auto stage = std::find_if(Pattern.Stages.begin(), Pattern.Stages.end(),
			[&](const auto& value) { return value.strStageId == key.strStageId; });
		if (item == m_TimelineItems.end() || stage == Pattern.Stages.end() || key.strPatternId != Pattern.strPatternId ||
			(!item->bEditable && !item->bEffectV2Binding))
		{ status = "A selected box is stale or read-only; the selection was preserved."; return false; }
		ROW row; row.item = *item;
		if (item->eOwner == DETAIL_OWNER::ANIMATION)
		{
			if (edit == TIMELINE_GROUP_EDIT::MOVE_BOXES && item->bLoopsToStageEnd)
			{ status = "Move the whole Stage to retain an Animation loop's end clock."; return false; }
			animationIds.insert(item->strStableId);
		}
		else if (item->bEffectV2Binding)
		{
			const auto* binding = ResolveEffectV2Binding(*effects, item->strStableId, item->strEffectV2ClipOccurrenceId);
			if (!binding) { status = "A selected V2 binding no longer exists."; return false; }
			row.v2 = *binding;
		}
		else if (item->eOwner == DETAIL_OWNER::EFFECT)
		{
			for (const auto& cue : stage->ProductCues) if (cue.strOccurrenceId == item->strStableId) row.effect = cue;
			if (!row.effect) { status = "A selected Effect no longer exists."; return false; }
		}
		else if (item->eOwner == DETAIL_OWNER::SOUND)
		{
			for (const auto& cue : sounds->Cues)
				if (cue.strPatternId == Pattern.strPatternId && cue.strStageId == stage->strStageId && cue.strOccurrenceId == item->strStableId) row.sound = cue;
			if (!row.sound) { status = "A selected Sound no longer exists."; return false; }
		}
		else if (item->eLane == TIMELINE_LANE::COLLIDER && item->strStableId == stage->strStageId + "/collider")
		{
			CBalanceTool::PATTERN_STAGE_EDIT draft;
			if (!m_pBalanceTool->Get_ValtanStageDraft(Pattern.strPatternId, stage->strStageId, draft, status)) return false;
			if (!draft.attackContacts.empty() || draft.playerResponse != "DAMAGE")
			{ status = "Per-contact and Grab colliders retain their typed owner editor."; return false; }
			row.collider = std::move(draft);
		}
		else { status = "Select Animation, Effect, Sound or ordinary Collider boxes for this group operation."; return false; }
		rows.push_back(std::move(row));
	}
	if (rows.empty()) { status = "Select timeline boxes first."; return false; }
	if (edit == TIMELINE_GROUP_EDIT::MOVE_BOXES && iWallDeltaMs == 0)
	{ status = "The selected boxes are already at that time."; return true; }
	const auto previousSelection = m_TimelineSelection;
	const auto previousStage = m_strSelectedStageId, previousId = m_strSelectedStableId;
	const auto previousOwner = m_eDetailOwner;
	const auto previousShakes = m_PatternShakes;
	const auto previousEffectIdentity = m_strEffectEditIdentity, previousV2Identity = m_strEffectV2BindingEditId;
	const auto previousV2Revision = m_iEffectV2CatalogRevision;
	const bool_t previousDraftDirty = m_bAuthoringDraftDirty;
	std::vector<TIMELINE_SELECTION> nextSelection;
	int64_t delta = iWallDeltaMs;
	const auto clearCollider = [&](const std::string& stageId, std::string& message) {
		CBalanceTool::PATTERN_STAGE_EDIT candidate, empty;
		if (!m_pBalanceTool->Get_ValtanStageDraft(Pattern.strPatternId, stageId, candidate, message)) return false;
		if (!candidate.colliderRemoveAdmitted) { message = "The selected Collider is a required gameplay owner and cannot be removed or transferred."; return false; }
		empty.hitShape = "NONE"; CopyValtanColliderFields(empty, candidate);
		return m_pBalanceTool->Set_ValtanStageDraft(Pattern.strPatternId, stageId, candidate, message);
	};
	const auto applied = CEffectV2Catalog::Get().Apply_BossValtanBindingDraftTransaction([&](std::string& v2Status) {
		return m_pAnimationTool->Apply_ValtanCompositionPatternSoundDraftTransaction([&](std::string& soundStatus) {
			return m_pBalanceTool->Apply_ValtanCompositionDraftTransaction([&](std::string& message) {
				// 선택 Animation을 이동·삭제하기 전에 함께 선택한 종속 cue를 분리한다.
				// 아래 선택 변경까지 세 소유 도구가 같은 rollback 범위에 들어간다.
				for (auto& row : rows)
				{
					const std::string linked = row.effect && !row.effect->bUsesStageClock ? row.effect->strClipOccurrenceId :
						row.sound ? row.sound->strClipOccurrenceId : row.v2 && row.v2->eClockBasis == EFFECT_V2_CLOCK_BASIS::CLIP_OCCURRENCE ? row.v2->strClipOccurrenceId : std::string{};
					row.detached = edit == TIMELINE_GROUP_EDIT::MOVE_BOXES && !linked.empty() && animationIds.contains(linked);
					if (edit != TIMELINE_GROUP_EDIT::DELETE_BOXES && !row.detached) continue;
					VALTAN_PATTERN_VIEW current;
					if (!m_pBalanceTool->Get_ValtanPatternDraft(Pattern.strPatternId, current, message)) return false;
					const auto stage = std::find_if(current.Stages.begin(), current.Stages.end(), [&](const auto& value) { return value.strStageId == row.item.strStageId; });
					if (stage == current.Stages.end()) return false;
					if (row.effect && !m_pBalanceTool->Remove_ValtanStageEffectCue(current.strPatternId, stage->strStageId, stage->strActionId,
						row.effect->strBindingId, row.effect->strOccurrenceId, row.effect->strEffectAssetId, row.effect->strClipOccurrenceId, message)) return false;
					if (row.sound)
					{
						VALTAN_PATTERN_SOUND_CUE_ROW_ID id; id.strBindingId = row.sound->strBindingId; id.strOccurrenceId = row.sound->strOccurrenceId;
						if (!m_pAnimationTool->Remove_ValtanCompositionPatternSound(current, *stage, id, message)) return false;
					}
					if (row.v2 && !CEffectV2Catalog::Get().Stage_RemoveBossValtanStageBinding(EFFECT_V2_STAGE_BINDING_KEY::From_Binding(*row.v2), message)) return false;
					if (row.collider && !clearCollider(stage->strStageId, message)) return false;
				}
				std::vector<const ROW*> animations;
				for (const auto& row : rows) if (row.item.eOwner == DETAIL_OWNER::ANIMATION) animations.push_back(&row);
				std::stable_sort(animations.begin(), animations.end(), [&](const auto* left, const auto* right) {
					return delta > 0 ? left->item.iStartMs > right->item.iStartMs : left->item.iStartMs < right->item.iStartMs;
				});
				for (const auto* row : animations)
				{
					VALTAN_PATTERN_VIEW current;
					if (!m_pBalanceTool->Get_ValtanPatternDraft(Pattern.strPatternId, current, message)) return false;
					const auto source = std::find_if(current.Stages.begin(), current.Stages.end(), [&](const auto& value) {
						return std::any_of(value.ClipOccurrences.begin(), value.ClipOccurrences.end(), [&](const auto& clip) { return clip.strClipOccurrenceId == row->item.strStableId; }); });
					if (source == current.Stages.end()) { message = "A selected Animation lost its exact occurrence."; return false; }
					if (edit == TIMELINE_GROUP_EDIT::DELETE_BOXES)
					{
						if (!Remove_AnimationOccurrence(current, *source, row->item.strStableId, message)) return false;
						continue;
					}
					Invalidate_TimelineCache(); Ensure_TimelineCache(&current);
					const int64_t center = int64_t(row->item.iStartMs) + delta + (row->item.iEndMs - row->item.iStartMs) / 2u;
					const TIMELINE_ITEM* targetBox = nullptr;
					for (const auto& box : m_TimelineItems)
						if (box.eLane == TIMELINE_LANE::STAGE && center >= box.iStartMs && center < box.iEndMs) targetBox = &box;
					if (!targetBox) { message = "Move the complete Animation selection onto an existing Stage."; return false; }
					const auto target = std::find_if(current.Stages.begin(), current.Stages.end(), [&](const auto& value) { return value.strStageId == targetBox->strStageId; });
					if (target == current.Stages.end()) return false;
					std::size_t index = 0u;
					for (const auto& box : m_TimelineItems)
						if (box.eLane == TIMELINE_LANE::ANIMATION && box.strStageId == target->strStageId && box.strStableId != row->item.strStableId &&
							center >= int64_t(box.iStartMs) + (box.iEndMs - box.iStartMs) / 2u) ++index;
					if (!Transfer_AnimationOccurrence(current, *source, *target, row->item.strStableId, index, message)) return false;
				}
				if (edit == TIMELINE_GROUP_EDIT::DELETE_BOXES) return true;
				VALTAN_PATTERN_VIEW current;
				if (!m_pBalanceTool->Get_ValtanPatternDraft(Pattern.strPatternId, current, message)) return false;
				Invalidate_TimelineCache(); Ensure_TimelineCache(&current);
				bool hasSnappedDelta = false;
				for (const auto* row : animations)
				{
					const auto moved = std::find_if(m_TimelineItems.begin(), m_TimelineItems.end(), [&](const auto& box) { return box.eOwner == DETAIL_OWNER::ANIMATION && box.strStableId == row->item.strStableId; });
					if (moved == m_TimelineItems.end()) return false;
					const int64_t actual = int64_t(moved->iStartMs) - row->item.iStartMs;
					if (hasSnappedDelta && actual != delta)
					{ message = "Animation boxes snap to Sequence boundaries; this destination changes the selected spacing. Move a contiguous block or its whole Stage."; return false; }
					delta = actual; hasSnappedDelta = true; nextSelection.push_back(Timeline_SelectionKey(*moved));
				}
				for (const auto& row : rows)
				{
					if (row.item.eOwner == DETAIL_OWNER::ANIMATION) continue;
					if (!m_pBalanceTool->Get_ValtanPatternDraft(Pattern.strPatternId, current, message)) return false;
					Invalidate_TimelineCache(); Ensure_TimelineCache(&current);
					const auto source = std::find_if(current.Stages.begin(), current.Stages.end(), [&](const auto& value) { return value.strStageId == row.item.strStageId; });
					if (source == current.Stages.end()) return false;
					const int64_t requested = int64_t(row.item.iStartMs) + delta;
					if (requested < 0 || requested > 600000) { message = "The selected boxes exceed the Pattern clock."; return false; }
					const TIMELINE_ITEM* targetBox = nullptr;
					for (const auto& box : m_TimelineItems)
						if (box.eLane == TIMELINE_LANE::STAGE && requested >= box.iStartMs && requested < box.iEndMs) targetBox = &box;
					if (!targetBox) { message = "Every moved box must start inside a destination Stage."; return false; }
					const auto target = std::find_if(current.Stages.begin(), current.Stages.end(), [&](const auto& value) { return value.strStageId == targetBox->strStageId; });
					if (target == current.Stages.end()) return false;
					TIMELINE_SELECTION next = Timeline_SelectionKey(row.item); next.strStageId = target->strStageId;
					const uint32_t local = static_cast<uint32_t>(requested - targetBox->iStartMs);
					if (row.detached)
					{
						const std::string clipId = row.effect ? row.effect->strClipOccurrenceId : row.sound ? row.sound->strClipOccurrenceId : row.v2->strClipOccurrenceId;
						const auto clip = std::find_if(target->ClipOccurrences.begin(), target->ClipOccurrences.end(), [&](const auto& value) { return value.strClipOccurrenceId == clipId; });
						if (clip == target->ClipOccurrences.end()) { message = "A selected cue did not land beside its moved Animation."; return false; }
						if (row.effect)
						{
							auto cue = *row.effect; cue.iPlaybackOffsetMs = ResolveEffectPlaybackOffsetMs(cue); cue.bHasPlaybackOffset = true;
							cue.strStageId = target->strStageId; cue.strActionId = target->strActionId; cue.iStageDurationMs = target->iDurationMs;
							cue.strBindingId = BuildNextCompositionEffectCueId(current, *target); cue.strOccurrenceId = cue.strBindingId + ".occurrence.01";
							if (!ValidateValtanEffectInvocationClock(m_pAnimationTool, *target, cue, message) ||
								!m_pBalanceTool->Add_ValtanStageEffectCue(current.strPatternId, target->strStageId, target->strActionId, cue, message)) return false;
							next.strStableId = cue.strOccurrenceId;
						}
						else if (row.sound)
						{
							const auto& cue = *row.sound; VALTAN_PATTERN_SOUND_CUE_ROW_ID id;
							if (!m_pAnimationTool->Add_ValtanCompositionPatternSound(current, *target, clipId, cue.strSoundEvent,
								cue.iStartMs, cue.eRepeatPolicy, id, message, cue.bHasPlaybackOffset ? std::optional<uint32_t>{cue.iPlaybackOffsetMs} : std::nullopt,
								cue.bHasPlaybackDuration ? std::optional<uint32_t>{cue.iPlaybackDurationMs} : std::nullopt)) return false;
							next.strStableId = id.strOccurrenceId;
						}
						else
						{
							auto cue = *row.v2; cue.strStageId = target->strStageId; cue.strActionId = target->strActionId;
							std::vector<std::string> ids;
							if (!Validate_EffectV2BindingClock(*target, cue, message) || !CEffectV2Catalog::Get().Stage_AppendBossValtanBindings({cue}, ids, message)) return false;
							cue.strBindingId = ids.front(); next.strStableId = BuildEffectV2BindingStableId(cue, cue.strClipOccurrenceId);
						}
					}
					else if (row.effect)
					{
						const int64_t end = int64_t(row.item.iEndMs) + delta;
						if (row.effect->bHasSourceEnd && (end <= requested || end > 600000)) { message = "The selected Effect end exceeds its clock."; return false; }
						if (!MoveValtanEffectInvocation(m_pAnimationTool, m_pBalanceTool, current, *source, *row.effect,
							m_TimelineItems, static_cast<uint32_t>(requested), next.strStageId, next.strStableId, message,
							row.effect->bHasSourceEnd ? std::optional<uint32_t>{static_cast<uint32_t>(end)} : std::nullopt)) return false;
					}
					else if (row.sound)
					{
						if (!Apply_SoundTimelinePlacement(current, *source, *row.sound, static_cast<uint32_t>(requested),
							row.sound->bHasPlaybackOffset ? row.sound->iPlaybackOffsetMs : 0u,
							row.sound->bHasPlaybackDuration ? std::optional<uint32_t>{row.sound->iPlaybackDurationMs} : std::nullopt, message, false)) return false;
						next.strStageId = m_strSelectedStageId;
					}
					else if (row.v2)
					{
						auto cue = *row.v2; cue.strStageId = target->strStageId; cue.strActionId = target->strActionId; cue.iStartMs = local;
						if (cue.eClockBasis == EFFECT_V2_CLOCK_BASIS::CLIP_OCCURRENCE)
						{
							const TIMELINE_ITEM* clipBox = nullptr;
							for (const auto& box : m_TimelineItems) if (box.eLane == TIMELINE_LANE::ANIMATION && box.strStageId == target->strStageId && requested >= box.iStartMs && requested < box.iEndMs) clipBox = &box;
							if (!clipBox) { message = "A moved linked V2 Effect needs an Animation at its start."; return false; }
							const auto clip = std::find_if(target->ClipOccurrences.begin(), target->ClipOccurrences.end(), [&](const auto& value) { return value.strClipOccurrenceId == clipBox->strStableId; });
							if (clip == target->ClipOccurrences.end() || !std::isfinite(clip->fPlayRate) || clip->fPlayRate <= 0.f) return false;
							cue.strClipOccurrenceId = clip->strClipOccurrenceId;
							cue.iStartMs = clip->iSourceStartMs + static_cast<uint32_t>(std::llround((requested - clipBox->iStartMs) * double(clip->fPlayRate)));
						}
						if (!Validate_EffectV2BindingClock(*target, cue, message) || !CEffectV2Catalog::Get().Stage_MoveBossValtanBinding(
							EFFECT_V2_STAGE_BINDING_KEY::From_Binding(*row.v2), cue.strStageId, cue.strActionId, cue.strClipOccurrenceId, cue.iStartMs, message)) return false;
						next.strStableId = BuildEffectV2BindingStableId(cue, cue.strClipOccurrenceId);
					}
					else if (row.collider)
					{
						CBalanceTool::PATTERN_STAGE_EDIT candidate;
						if (!m_pBalanceTool->Get_ValtanStageDraft(current.strPatternId, target->strStageId, candidate, message)) return false;
						uint32_t first = 0u, last = 0u;
						if (!ResolveValtanColliderScheduleEndpoints(*row.collider, first, last, message)) return false;
						const uint64_t end = uint64_t(local) + last - first;
						if (local >= candidate.durationMs || end > candidate.durationMs || (!row.collider->hasHitActivation && end == candidate.durationMs))
						{ message = "The entire moved Collider schedule must fit its destination Stage."; return false; }
						if (source->strStageId == target->strStageId)
						{
							if (!TranslateValtanColliderSchedule(candidate, int64_t(local) - first, message)) return false;
						}
						else
						{
							if (!clearCollider(source->strStageId, message) || !BuildValtanColliderPasteDraft(*row.collider, local, candidate, message)) return false;
						}
						if (!m_pBalanceTool->Set_ValtanStageDraft(current.strPatternId, target->strStageId, candidate, message)) return false;
						next.strStableId = target->strStageId + "/collider";
					}
					nextSelection.push_back(std::move(next));
				}
				return true;
			}, soundStatus);
		}, v2Status);
	}, status);
	if (!applied)
	{
		m_TimelineSelection = previousSelection; m_strSelectedStageId = previousStage; m_strSelectedStableId = previousId; m_eDetailOwner = previousOwner;
		m_PatternShakes = previousShakes;
		m_strEffectEditIdentity = previousEffectIdentity; m_strEffectV2BindingEditId = previousV2Identity;
		m_iEffectV2CatalogRevision = previousV2Revision; m_bAuthoringDraftDirty = previousDraftDirty;
		Invalidate_EffectivePatternCache(); Invalidate_TimelineCache();
		status = "Group edit rejected; all selected boxes were preserved. " + status; return false;
	}
	m_TimelineSelection = std::move(nextSelection);
	if (!m_TimelineSelection.empty())
	{
		const auto& focus = m_TimelineSelection.back(); m_strSelectedStageId = focus.strStageId; m_strSelectedStableId = focus.strStableId; m_eDetailOwner = focus.eOwner;
	}
	else { m_strSelectedStageId = previousStage; m_strSelectedStableId = previousStage; m_eDetailOwner = DETAIL_OWNER::GAMEPLAY_STAGE; }
	m_strEffectEditIdentity.clear(); m_strEffectV2BindingEditId.clear();
	m_iEffectV2CatalogRevision = CEffectV2Catalog::Get().Get_Revision(); m_bAuthoringDraftDirty = m_pBalanceTool->Is_ValtanDraftDirty();
	status = std::string(edit == TIMELINE_GROUP_EDIT::MOVE_BOXES ? "Moved " : "Deleted ") + std::to_string(rows.size()) +
		" selected boxes together" + (edit == TIMELINE_GROUP_EDIT::MOVE_BOXES ? " by " + std::to_string(delta) + " ms" : std::string{}) + ". Save commits the staged values.";
	Refresh_PatternLocalPreviewAfterMutation(&Pattern, status); return true;
}

bool_t Client::CValtanActionWorkbench::Duplicate_SelectedTimelineBox(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	std::string& strOutStatus)
{
	if (m_TimelineSelection.size() > 1u)
		return Execute_CompositionEdit(COMPOSITION_EDIT_COMMAND::DUPLICATE_SELECTION, strOutStatus);
	if (m_eDetailOwner == DETAIL_OWNER::GAMEPLAY_STAGE && m_strSelectedStableId == Stage.strStageId + "/collider")
		return Execute_CompositionEdit(COMPOSITION_EDIT_COMMAND::DUPLICATE_SELECTION, strOutStatus);
	const auto SelectedV2 = std::find_if(
		m_TimelineItems.begin(), m_TimelineItems.end(),
		[this, &Pattern, &Stage](const TIMELINE_ITEM& Item)
		{
			return Item.bEffectV2Binding &&
				Item.strPatternId == Pattern.strPatternId &&
				Item.strStageId == Stage.strStageId &&
				Item.strStableId == m_strSelectedStableId;
		});
	if (m_TimelineItems.end() != SelectedV2)
	{
		const std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> pSnapshot =
			CEffectV2Catalog::Get().Get_Snapshot();
		const EFFECT_V2_BINDING* const pSourceBinding = nullptr == pSnapshot ?
			nullptr : ResolveEffectV2Binding(
				*pSnapshot, SelectedV2->strStableId,
				SelectedV2->strEffectV2ClipOccurrenceId);
		if (nullptr == pSourceBinding)
		{
			strOutStatus =
				"V2 Duplicate lost its exact saved binding row. Refresh Effect Resources and select the box again.";
			return false;
		}
		if (0u == Stage.iDurationMs)
		{
			strOutStatus =
				"V2 Duplicate requires a non-empty Stage clock.";
			return false;
		}
		const uint32_t iMaximumClockMs = EFFECT_V2_CLOCK_BASIS::STAGE == pSourceBinding->eClockBasis ?
			Stage.iDurationMs - 1u : 600000u;
		const uint32_t iSourceClockMs = (std::min)(
			SelectedV2->iEffectV2BindingStartMs, iMaximumClockMs);
		const uint32_t iDuplicateStartMs = iSourceClockMs < iMaximumClockMs ?
			iSourceClockMs + 1u : iSourceClockMs - 1u;
		EFFECT_V2_BINDING Candidate = *pSourceBinding;
		Candidate.iStartMs = iDuplicateStartMs;
		if (!Validate_EffectV2BindingClock(Stage, Candidate, strOutStatus))
			return false;
		const EFFECT_V2_STAGE_BINDING_KEY Key =
			EFFECT_V2_STAGE_BINDING_KEY::From_StageBinding(*pSourceBinding);
		if (!CEffectV2Catalog::Get().Stage_DuplicateBossValtanStageBinding(
				Key, iDuplicateStartMs, strOutStatus))
		{
			return false;
		}
		m_iEffectV2CatalogRevision =
			CEffectV2Catalog::Get().Get_Revision();
		m_eDetailOwner = DETAIL_OWNER::EFFECT;
		const auto UpdatedSnapshot = CEffectV2Catalog::Get().Get_Snapshot();
		if (UpdatedSnapshot)
		{
			for (const EFFECT_V2_BINDING& Binding : UpdatedSnapshot->Get_BossValtanBindings())
			{
				const bool Existed = std::any_of(pSnapshot->Get_BossValtanBindings().begin(),
					pSnapshot->Get_BossValtanBindings().end(), [&Binding](const EFFECT_V2_BINDING& Previous)
					{ return Previous.strBindingId == Binding.strBindingId; });
				if (!Existed)
				{
					m_strSelectedStableId = BuildEffectV2BindingStableId(
						Binding, SelectedV2->strEffectV2ClipOccurrenceId);
					break;
				}
			}
		}
		Refresh_EffectV2LocalPreviewAfterMutation(&Pattern, strOutStatus);
		return true;
	}

	switch (m_eDetailOwner)
	{
	case DETAIL_OWNER::ANIMATION:
		return Duplicate_AnimationOccurrence(
			Pattern, Stage, m_strSelectedStableId, strOutStatus);
	case DETAIL_OWNER::EFFECT:
	{
		if (nullptr == m_pBalanceTool)
		{
			strOutStatus = "Effect Duplicate requires the typed Pattern owner.";
			return false;
		}
		const std::size_t iMatchCount = static_cast<std::size_t>(std::count_if(
			Stage.ProductCues.begin(), Stage.ProductCues.end(),
			[this](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue)
			{ return Cue.strOccurrenceId == m_strSelectedStableId; }));
		const auto Found = std::find_if(
			Stage.ProductCues.begin(), Stage.ProductCues.end(),
			[this](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue)
			{ return Cue.strOccurrenceId == m_strSelectedStableId; });
		if (1u != iMatchCount || Stage.ProductCues.end() == Found)
		{
			strOutStatus =
				"Effect Duplicate requires one exact selected invocation box.";
			return false;
		}
		VALTAN_PRODUCT_EFFECT_CUE_VIEW Duplicate = *Found;
		Duplicate.iPlaybackOffsetMs = ResolveEffectPlaybackOffsetMs(*Found);
		Duplicate.bHasPlaybackOffset = true;
		Duplicate.strBindingId = BuildNextCompositionEffectCueId(Pattern, Stage);
		if (Duplicate.strBindingId.empty())
		{
			strOutStatus =
				"Effect Duplicate rejected: no free stable invocation ID is available.";
			return false;
		}
		Duplicate.strOccurrenceId =
			Duplicate.strBindingId + ".occurrence.01";
		if (!m_pBalanceTool->Add_ValtanStageEffectCue(
				Pattern.strPatternId, Stage.strStageId, Stage.strActionId,
				Duplicate, strOutStatus))
		{
			return false;
		}
		m_eDetailOwner = DETAIL_OWNER::EFFECT;
		m_strSelectedStableId = Duplicate.strOccurrenceId;
		m_strEffectEditIdentity.clear();
		Invalidate_TimelineCache();
		strOutStatus =
			"Duplicated the selected Effect box at the same exact occurrence time; drag or trim the copy, then Save.";
		Refresh_PatternLocalPreviewAfterMutation(&Pattern, strOutStatus);
		return true;
	}
	case DETAIL_OWNER::SOUND:
	{
		if (nullptr == m_pAnimationTool)
		{
			strOutStatus = "Sound Duplicate requires the typed Sound owner.";
			return false;
		}
		bool_t bDirty = false;
		std::string DraftStatus;
		const VALTAN_PATTERN_SOUND_CUE_DOCUMENT* const pSounds =
			m_pAnimationTool->Get_ValtanCompositionPatternSoundDraft(
				bDirty, DraftStatus);
		if (nullptr == pSounds)
		{
			strOutStatus = std::move(DraftStatus);
			return false;
		}
		const auto Found = std::find_if(
			pSounds->Cues.begin(), pSounds->Cues.end(),
			[this, &Pattern, &Stage](const VALTAN_PATTERN_SOUND_CUE& Cue)
			{
				return Cue.strOccurrenceId == m_strSelectedStableId &&
					Cue.strPatternId == Pattern.strPatternId &&
					Cue.strStageId == Stage.strStageId &&
					Cue.strActionId == Stage.strActionId;
			});
		if (pSounds->Cues.end() == Found)
		{
			strOutStatus =
				"Sound Duplicate requires one exact selected Sound box.";
			return false;
		}
		VALTAN_PATTERN_SOUND_CUE_ROW_ID Created;
		if (!m_pAnimationTool->Add_ValtanCompositionPatternSound(
				Pattern, Stage, Found->strClipOccurrenceId,
				Found->strSoundEvent, Found->iStartMs,
				Found->eRepeatPolicy, Created, strOutStatus,
				Found->bHasPlaybackOffset ? std::optional<uint32_t>{Found->iPlaybackOffsetMs} : std::nullopt,
				Found->bHasPlaybackDuration ? std::optional<uint32_t>{Found->iPlaybackDurationMs} : std::nullopt))
		{
			return false;
		}
		m_eDetailOwner = DETAIL_OWNER::SOUND;
		m_strSelectedStableId = Created.strOccurrenceId;
		Refresh_PatternLocalPreviewAfterMutation(&Pattern, strOutStatus);
		m_strSoundStatus = strOutStatus;
		return true;
	}
	default:
		strOutStatus =
			"Duplicate is available for a selected Animation, Effect, or Sound box.";
		return false;
	}
}

bool_t Client::CValtanActionWorkbench::Reorder_SelectedStages(
	const VALTAN_PATTERN_VIEW& Pattern, const int32_t direction, std::string& status)
{
	if (!m_pBalanceTool || !m_pAnimationTool || !Can_MutateValtanView(m_eAdmission) ||
		!Pattern.bManualServerAudition || m_pBalanceTool->Is_ValtanSaveJobBlockingAuthoring() ||
		m_pBalanceTool->Is_ServerRuntimeSetPublishRunning() ||
		m_pAnimationTool->Is_ValtanCompositionPatternTransactionActive())
	{ status = "Earlier/Later requires an admitted, idle manual Pattern."; return false; }
	if (!direction || Pattern.Stages.empty())
	{ status = "Select Stage or Animation boxes and a move direction."; return false; }
	const VALTAN_PATTERN_VIEW original = Pattern;
	std::set<std::string> selected;
	const auto selectOwner = [&](const std::string& patternId, const std::string& stageId,
		const DETAIL_OWNER owner, const std::string& stableId) {
		const auto stage = std::find_if(original.Stages.begin(), original.Stages.end(),
			[&](const auto& row) { return row.strStageId == stageId; });
		if (patternId != original.strPatternId || stage == original.Stages.end()) return false;
		const bool stageBox = owner == DETAIL_OWNER::GAMEPLAY_STAGE && stableId == stageId;
		const bool animationBox = owner == DETAIL_OWNER::ANIMATION && std::any_of(
			stage->ClipOccurrences.begin(), stage->ClipOccurrences.end(),
			[&](const auto& clip) { return clip.strClipOccurrenceId == stableId; });
		if (!stageBox && !animationBox) return false;
		selected.insert(stageId);
		return true;
	};
	if (m_TimelineSelection.empty())
	{
		if (!selectOwner(m_strSelectedPatternId, m_strSelectedStageId, m_eDetailOwner, m_strSelectedStableId))
		{ status = "Select Stage or Animation boxes for Earlier/Later."; return false; }
	}
	else for (const auto& key : m_TimelineSelection)
		if (!selectOwner(key.strPatternId, key.strStageId, key.eOwner, key.strStableId))
		{ status = "Earlier/Later reorders Stage and Animation selections. Select only those boxes; draft and selection preserved."; return false; }
	if (selected.contains(direction < 0 ? original.Stages.front().strStageId : original.Stages.back().strStageId))
	{ status = "Selected Stages are already at that edge; draft and selection preserved."; return false; }

	VALTAN_PATTERN_VIEW candidate;
	const bool applied = m_pBalanceTool->Apply_ValtanCompositionDraftTransaction([&](std::string& diagnostic) {
		// 이동 방향 쪽부터 방문해 연속 선택 묶음마다 이웃 하나만 넘도록 한다.
		std::vector<std::string> order;
		for (const auto& row : original.Stages) order.push_back(row.strStageId);
		if (direction < 0)
		{
			for (std::size_t index = 1u; index < order.size(); ++index)
				if (selected.contains(order[index]) && !selected.contains(order[index - 1u]))
				{
					if (!m_pBalanceTool->Move_ValtanManualStage(original.strPatternId, order[index], order[index - 1u], true, diagnostic)) return false;
					std::swap(order[index], order[index - 1u]);
				}
		}
		else
		{
			for (std::size_t index = order.size() - 1u; index > 0u; --index)
				if (selected.contains(order[index - 1u]) && !selected.contains(order[index]))
				{
					if (!m_pBalanceTool->Move_ValtanManualStage(original.strPatternId, order[index - 1u], order[index], false, diagnostic)) return false;
					std::swap(order[index - 1u], order[index]);
				}
		}
		return m_pBalanceTool->Get_ValtanPatternDraft(original.strPatternId, candidate, diagnostic);
	}, status);
	if (!applied)
	{ status = "Earlier/Later rejected; the entire draft and selection were preserved. " + status; return false; }
	m_BossPatternOutcomeOverrides.clear(); ++m_iBossPatternRouteGeneration;
	m_strEffectEditIdentity.clear(); m_bAuthoringDraftDirty = m_pBalanceTool->Is_ValtanDraftDirty();
	Invalidate_EffectivePatternCache(); Invalidate_TimelineCache();
	status = "Moved " + std::to_string(selected.size()) + (direction < 0 ? " Stage(s) one slot earlier" : " Stage(s) one slot later") +
		" with their Animation, Effect, Sound and Collider. Selection and local timing are preserved. Save when ready.";
	Refresh_PatternLocalPreviewAfterMutation(&candidate, status);
	return true;
}

bool_t Client::CValtanActionWorkbench::Move_SelectedStages(
	const VALTAN_PATTERN_VIEW& Pattern, const int64_t deltaMs, std::string& status)
{
	if (!m_pBalanceTool || !Can_MutateValtanView(m_eAdmission) || !Pattern.bManualServerAudition)
	{ status = "Stage drag requires an admitted manual Pattern."; return false; }
	std::set<std::string> selected;
	for (const auto& key : m_TimelineSelection)
		if (key.strPatternId == Pattern.strPatternId && key.eOwner == DETAIL_OWNER::GAMEPLAY_STAGE && key.strStableId == key.strStageId)
			selected.insert(key.strStageId);
	if (selected.empty() && m_eDetailOwner == DETAIL_OWNER::GAMEPLAY_STAGE && m_strSelectedStableId == m_strSelectedStageId)
		selected.insert(m_strSelectedStageId);
	if (selected.empty()) { status = "Select a Stage box to reorder it."; return false; }
	for (const auto& key : m_TimelineSelection)
		if (key.strPatternId != Pattern.strPatternId || !selected.contains(key.strStageId))
		{ status = "Stage drag reorders whole Stages and their owned boxes. Deselect boxes belonging to other Stages, or select their Stage too; draft preserved."; return false; }
	const VALTAN_PATTERN_VIEW original = Pattern;
	std::vector<std::string> moved, remaining, before;
	std::vector<int64_t> remainingMidpoints;
	int64_t cursor = 0, firstMidpoint = -1;
	for (const auto& row : original.Stages)
	{
		before.push_back(row.strStageId);
		if (selected.contains(row.strStageId))
		{
			moved.push_back(row.strStageId);
			if (firstMidpoint < 0) firstMidpoint = cursor + row.iDurationMs / 2u;
		}
		else { remaining.push_back(row.strStageId); remainingMidpoints.push_back(cursor + row.iDurationMs / 2u); }
		cursor += row.iDurationMs;
	}
	if (moved.size() != selected.size()) { status = "A selected Stage changed before the drag completed; draft preserved."; return false; }
	if (!deltaMs || remaining.empty()) { status = "Stage order is unchanged."; return false; }
	const int64_t dropMidpoint = firstMidpoint + deltaMs;
	std::size_t insertion = 0u;
	while (insertion < remainingMidpoints.size() && dropMidpoint >= remainingMidpoints[insertion]) ++insertion;
	auto after = remaining;
	after.insert(after.begin() + static_cast<std::ptrdiff_t>(insertion), moved.begin(), moved.end());
	if (after == before) { status = "Stage order is unchanged; drag across another Stage to reorder."; return false; }
	VALTAN_PATTERN_VIEW candidate;
	const bool applied = m_pBalanceTool->Apply_ValtanCompositionDraftTransaction([&](std::string& diagnostic) {
		std::string anchor = insertion < remaining.size() ? remaining[insertion] : remaining.back();
		const bool beforeAnchor = insertion < remaining.size();
		for (const auto& id : moved)
		{
			if (!m_pBalanceTool->Move_ValtanManualStage(original.strPatternId, id, anchor, beforeAnchor, diagnostic)) return false;
			if (!beforeAnchor) anchor = id;
		}
		return m_pBalanceTool->Get_ValtanPatternDraft(original.strPatternId, candidate, diagnostic);
	}, status);
	if (!applied) { status = "Stage drag rejected; the entire draft was preserved. " + status; return false; }
	m_BossPatternOutcomeOverrides.clear(); ++m_iBossPatternRouteGeneration;
	m_strEffectEditIdentity.clear(); m_bAuthoringDraftDirty = m_pBalanceTool->Is_ValtanDraftDirty();
	Invalidate_EffectivePatternCache(); Invalidate_TimelineCache();
	status = "Reordered " + std::to_string(moved.size()) + " Stage(s) with their Animation, Effect, Sound and Collider; IDs and local timing are preserved. Save when ready.";
	Refresh_PatternLocalPreviewAfterMutation(&candidate, status);
	return true;
}

bool_t Client::CValtanActionWorkbench::Delete_SelectedStages(
	const VALTAN_PATTERN_VIEW& Pattern, std::string& strOutStatus)
{
	if (!m_pBalanceTool || !m_pAnimationTool) return false;
	const auto savedSelection = m_TimelineSelection;
	const auto savedShakes = m_PatternShakes;
	std::set<std::string> stageIds;
	for (const auto& key : savedSelection)
		if (key.strPatternId == Pattern.strPatternId && key.eOwner == DETAIL_OWNER::GAMEPLAY_STAGE && key.strStableId == key.strStageId)
			stageIds.insert(key.strStageId);
	for (const auto& key : savedSelection)
		if (!stageIds.contains(key.strStageId))
		{ strOutStatus = "Stage group Delete requires whole Stages and their owned boxes; remove unrelated box selections or select their Stage. Draft preserved."; return false; }
	if (stageIds.empty()) return false;
	const VALTAN_PATTERN_VIEW original = Pattern;
	const auto savedPattern = m_strSelectedPatternId, savedStage = m_strSelectedStageId, savedStable = m_strSelectedStableId;
	const auto savedOwner = m_eDetailOwner;
	const auto savedEffectIdentity = m_strEffectEditIdentity;
	const auto savedOverrides = m_BossPatternOutcomeOverrides;
	const auto savedRouteGeneration = m_iBossPatternRouteGeneration;
	const auto savedEffectRevision = m_iEffectV2CatalogRevision;
	const auto savedDirty = m_bAuthoringDraftDirty;
	VALTAN_PATTERN_VIEW candidate;
	const bool applied = CEffectV2Catalog::Get().Apply_BossValtanBindingDraftTransaction([&](std::string& effectStatus) {
		return m_pAnimationTool->Apply_ValtanCompositionPatternSoundDraftTransaction([&](std::string& soundStatus) {
			return m_pBalanceTool->Apply_ValtanCompositionDraftTransaction([&](std::string& status) {
				for (const auto& source : original.Stages)
				{
					if (!stageIds.contains(source.strStageId)) continue;
					if (!m_pBalanceTool->Get_ValtanPatternDraft(original.strPatternId, candidate, status)) return false;
					const auto stage = std::find_if(candidate.Stages.begin(), candidate.Stages.end(), [&](const auto& row) { return row.strStageId == source.strStageId; });
					if (stage == candidate.Stages.end() || !Delete_SelectedStage(candidate, *stage, status, false)) return false;
				}
				return m_pBalanceTool->Get_ValtanPatternDraft(original.strPatternId, candidate, status);
			}, soundStatus);
		}, effectStatus);
	}, strOutStatus);
	if (!applied)
	{
		m_PatternShakes = savedShakes;
		m_TimelineSelection = savedSelection; m_strSelectedPatternId = savedPattern; m_strSelectedStageId = savedStage; m_strSelectedStableId = savedStable; m_eDetailOwner = savedOwner;
		m_strEffectEditIdentity = savedEffectIdentity; m_BossPatternOutcomeOverrides = savedOverrides;
		m_iBossPatternRouteGeneration = savedRouteGeneration; m_iEffectV2CatalogRevision = savedEffectRevision;
		m_bAuthoringDraftDirty = savedDirty;
		Invalidate_EffectivePatternCache(); Invalidate_TimelineCache();
		strOutStatus = "Stage group Delete rejected; all drafts were preserved. " + strOutStatus;
		return false;
	}
	strOutStatus = "Deleted " + std::to_string(stageIds.size()) + " selected Stage(s) and their owned resources. If all were selected, the final Stage remains empty. Save when ready.";
	Refresh_PatternLocalPreviewAfterMutation(&candidate, strOutStatus);
	return true;
}

bool_t Client::CValtanActionWorkbench::Delete_SelectedStage(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	std::string& strOutStatus, const bool_t bRefreshPreview)
{
	if (!m_pBalanceTool || !m_pAnimationTool || !Can_MutateValtanView(m_eAdmission) ||
		m_pBalanceTool->Is_ValtanSaveJobBlockingAuthoring() ||
		m_pBalanceTool->Is_ServerRuntimeSetPublishRunning() ||
		m_pAnimationTool->Is_ValtanCompositionPatternTransactionActive())
	{
		strOutStatus = "Stage Delete requires an admitted, idle draft.";
		return false;
	}
	// 값 복사본은 Balance transaction과 cache/preview 갱신 뒤에도 살아 있다.
	const VALTAN_PATTERN_VIEW original = Pattern;
	const VALTAN_STAGE_VIEW removed = Stage;
	if (!m_bPatternShakesReady) { strOutStatus = "Stage Delete requires the Camera Shake dependency source; draft preserved. " + m_strShakeStatus; return false; }
	auto candidateShakes = m_PatternShakes;
	if (!CValtanPatternShakeCueDocument::Remove_StageDraft(candidateShakes, original.strPatternId, removed.strStageId, strOutStatus)) return false;
	bool_t soundDirty = false;
	const auto* sounds = m_pAnimationTool->Get_ValtanCompositionPatternSoundDraft(soundDirty, strOutStatus);
	const auto effects = CEffectV2Catalog::Get().Get_Snapshot();
	if (!sounds || !effects || !effects->Is_Ready() || !effects->Can_MutateBossValtanBindings())
	{
		strOutStatus = "Stage Delete requires the complete Sound and Effect resource views; draft preserved. " + strOutStatus;
		return false;
	}
	std::vector<VALTAN_PATTERN_SOUND_CUE_ROW_ID> soundRows;
	std::vector<EFFECT_V2_STAGE_BINDING_KEY> effectRows;
	for (const auto& cue : sounds->Cues)
		if (cue.strPatternId == original.strPatternId && cue.strStageId == removed.strStageId)
		{
			if (cue.strActionId != removed.strActionId)
			{ strOutStatus = "Stage Delete found a stale Sound owner; draft preserved."; return false; }
			soundRows.push_back({cue.strBindingId, cue.strOccurrenceId});
		}
	for (const auto& binding : effects->Get_BossValtanBindings())
		if (binding.strPatternId == original.strPatternId && binding.strStageId == removed.strStageId)
		{
			if (binding.strActionId != removed.strActionId)
			{ strOutStatus = "Stage Delete found a stale Effect owner; draft preserved."; return false; }
			effectRows.push_back(EFFECT_V2_STAGE_BINDING_KEY::From_Binding(binding));
		}
	VALTAN_PATTERN_VIEW candidate;
	const bool applied = m_pAnimationTool->Apply_ValtanCompositionPatternSoundDraftTransaction(
		[&](std::string& diagnostic) {
			return m_pBalanceTool->Apply_ValtanCompositionDraftTransaction([&](std::string& message) {
				if (!m_pBalanceTool->Remove_ValtanManualStage(original.strPatternId, removed.strStageId, message)) return false;
				for (const auto& row : soundRows)
					if (!m_pAnimationTool->Remove_ValtanCompositionPatternSound(original, removed, row, message)) return false;
				if (!m_pBalanceTool->Get_ValtanPatternDraft(original.strPatternId, candidate, message)) return false;
				// 마지막에 V2 후보 하나를 commit한다. 어느 단계든 실패하면 Balance와 Sound는
				// 무관한 편집을 포함한 직전 전체 draft로 rollback한다.
				return CEffectV2Catalog::Get().Stage_RemoveBossValtanBindings(effectRows, message);
			}, diagnostic);
		}, strOutStatus);
	if (!applied) return false;
	m_PatternShakes = std::move(candidateShakes);
	const auto previous = std::find_if(original.Stages.begin(), original.Stages.end(),
		[&](const auto& row) { return row.strStageId == removed.strStageId; });
	const std::size_t next = (std::min)(static_cast<std::size_t>(previous - original.Stages.begin()), candidate.Stages.size() - 1u);
	m_strSelectedPatternId = original.strPatternId;
	m_strSelectedStageId = candidate.Stages[next].strStageId;
	m_strSelectedStableId = m_strSelectedStageId;
	m_eDetailOwner = DETAIL_OWNER::GAMEPLAY_STAGE;
	m_TimelineSelection.clear();
	m_strEffectEditIdentity.clear();
	m_BossPatternOutcomeOverrides.clear();
	++m_iBossPatternRouteGeneration;
	m_iEffectV2CatalogRevision = CEffectV2Catalog::Get().Get_Revision();
	Invalidate_EffectivePatternCache();
	Invalidate_TimelineCache();
	m_bAuthoringDraftDirty = m_pBalanceTool->Is_ValtanDraftDirty();
	strOutStatus = (original.Stages.size() == 1u ? "Cleared the final Stage; an empty 1000 ms Stage remains. " :
		"Deleted Stage " + removed.strStageId + "; following Stages close the gap. ") +
		std::string("Its Animation, Collider, Effect, Sound and Camera resources were removed together. Save when ready.");
	if (bRefreshPreview) Refresh_PatternLocalPreviewAfterMutation(&candidate, strOutStatus);
	return true;
}

bool_t Client::CValtanActionWorkbench::Delete_SelectedTimelineBox(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	std::string& strOutStatus)
{
	if (m_TimelineSelection.size() > 1u)
		return Apply_TimelineGroupEdit(Pattern, TIMELINE_GROUP_EDIT::DELETE_BOXES, 0, strOutStatus);
	if (m_eDetailOwner == DETAIL_OWNER::GAMEPLAY_STAGE && m_strSelectedStableId == Stage.strStageId + "/collider")
	{
		const auto previous = m_TimelineSelection;
		m_TimelineSelection = {{Pattern.strPatternId, Stage.strStageId, m_strSelectedStableId, DETAIL_OWNER::GAMEPLAY_STAGE}};
		const bool applied = Apply_TimelineGroupEdit(Pattern, TIMELINE_GROUP_EDIT::DELETE_BOXES, 0, strOutStatus);
		if (!applied) m_TimelineSelection = previous;
		return applied;
	}
	const auto SelectedV2 = std::find_if(
		m_TimelineItems.begin(), m_TimelineItems.end(),
		[this, &Pattern, &Stage](const TIMELINE_ITEM& Item)
		{
			return Item.bEffectV2Binding &&
				Item.strPatternId == Pattern.strPatternId &&
				Item.strStageId == Stage.strStageId &&
				Item.strStableId == m_strSelectedStableId;
		});
	if (m_TimelineItems.end() != SelectedV2)
	{
		const std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> pSnapshot =
			CEffectV2Catalog::Get().Get_Snapshot();
		const EFFECT_V2_BINDING* const pSourceBinding = nullptr == pSnapshot ?
			nullptr : ResolveEffectV2Binding(
				*pSnapshot, SelectedV2->strStableId,
				SelectedV2->strEffectV2ClipOccurrenceId);
		if (nullptr == pSourceBinding)
		{
			strOutStatus =
				"V2 Delete lost its exact saved binding row. Refresh Effect Resources and select the box again.";
			return false;
		}
		const EFFECT_V2_STAGE_BINDING_KEY Key =
			EFFECT_V2_STAGE_BINDING_KEY::From_StageBinding(*pSourceBinding);
		if (!CEffectV2Catalog::Get().Stage_RemoveBossValtanStageBinding(
				Key, strOutStatus))
		{
			return false;
		}
		m_iEffectV2CatalogRevision =
			CEffectV2Catalog::Get().Get_Revision();
		m_strSelectedStableId = Stage.strStageId;
		m_strEffectEditIdentity.clear();
		Refresh_EffectV2LocalPreviewAfterMutation(&Pattern, strOutStatus);
		return true;
	}

	switch (m_eDetailOwner)
	{
	case DETAIL_OWNER::ANIMATION:
	{
		const std::string patternId = Pattern.strPatternId;
		if (!Remove_AnimationOccurrence(Pattern, Stage, m_strSelectedStableId, strOutStatus))
			return false;
		VALTAN_PATTERN_VIEW candidate;
		std::string draftStatus;
		if (m_pBalanceTool->Get_ValtanPatternDraft(patternId, candidate, draftStatus))
			Refresh_PatternLocalPreviewAfterMutation(&candidate, strOutStatus);
		return true;
	}
	case DETAIL_OWNER::EFFECT:
	{
		if (nullptr == m_pBalanceTool)
		{
			strOutStatus = "Effect Delete requires the typed Pattern owner.";
			return false;
		}
		const auto Found = std::find_if(
			Stage.ProductCues.begin(), Stage.ProductCues.end(),
			[this](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue)
			{ return Cue.strOccurrenceId == m_strSelectedStableId; });
		if (Stage.ProductCues.end() == Found)
		{
			strOutStatus = "Effect Delete requires one exact invocation box.";
			return false;
		}
		if (!m_pBalanceTool->Remove_ValtanStageEffectCue(
				Pattern.strPatternId, Stage.strStageId, Stage.strActionId,
				Found->strBindingId, Found->strOccurrenceId,
				Found->strEffectAssetId, Found->strClipOccurrenceId,
				strOutStatus))
		{
			return false;
		}
		m_strSelectedStableId = Stage.strStageId;
		m_strEffectEditIdentity.clear();
		Refresh_PatternLocalPreviewAfterMutation(&Pattern, strOutStatus);
		return true;
	}
	case DETAIL_OWNER::SOUND:
	{
		if (nullptr == m_pAnimationTool)
		{
			strOutStatus = "Sound Delete requires the typed Sound owner.";
			return false;
		}
		bool_t bDirty = false;
		std::string DraftStatus;
		const VALTAN_PATTERN_SOUND_CUE_DOCUMENT* const pSounds =
			m_pAnimationTool->Get_ValtanCompositionPatternSoundDraft(
				bDirty, DraftStatus);
		if (nullptr == pSounds)
		{
			strOutStatus = std::move(DraftStatus);
			return false;
		}
		const auto Found = std::find_if(
			pSounds->Cues.begin(), pSounds->Cues.end(),
			[this, &Pattern, &Stage](const VALTAN_PATTERN_SOUND_CUE& Cue)
			{
				return Cue.strOccurrenceId == m_strSelectedStableId &&
					Cue.strPatternId == Pattern.strPatternId &&
					Cue.strStageId == Stage.strStageId &&
					Cue.strActionId == Stage.strActionId;
			});
		if (pSounds->Cues.end() == Found)
		{
			strOutStatus = "Sound Delete requires one exact Sound box.";
			return false;
		}
		VALTAN_PATTERN_SOUND_CUE_ROW_ID RowId;
		RowId.strBindingId = Found->strBindingId;
		RowId.strOccurrenceId = Found->strOccurrenceId;
		if (!m_pAnimationTool->Remove_ValtanCompositionPatternSound(
				Pattern, Stage, RowId, strOutStatus))
		{
			return false;
		}
		m_strSelectedStableId = Stage.strStageId;
		Refresh_PatternLocalPreviewAfterMutation(&Pattern, strOutStatus);
		m_strSoundStatus = strOutStatus;
		return true;
	}
	case DETAIL_OWNER::GAMEPLAY_STAGE:
	{
		const std::string CounterStableId =
			BuildBranchStableId(Stage.strStageId, "COUNTER_HIT");
		if (m_strSelectedStableId != CounterStableId ||
			nullptr == m_pBalanceTool)
		{
			strOutStatus =
				"Delete requires one selected Counter box in the Logic lane.";
			return false;
		}
		CBalanceTool::VALTAN_COUNTER_WINDOW_EDIT Counter;
		if (!m_pBalanceTool->Get_ValtanCounterWindowDraft(
				Pattern.strPatternId, Stage.strStageId, Counter, strOutStatus) ||
			!Counter.enabled)
		{
			if (strOutStatus.empty())
				strOutStatus = "The selected Counter box no longer exists.";
			return false;
		}
		Counter.enabled = false;
		if (!m_pBalanceTool->Set_ValtanCounterWindowDraft(
				Pattern.strPatternId, Stage.strStageId, Counter, strOutStatus))
		{
			return false;
		}
		m_strSelectedStableId = Stage.strStageId;
		Invalidate_TimelineCache();
		return true;
	}
	default:
		strOutStatus =
			"Delete is available for a selected Animation, Effect, Sound, or Counter box.";
		return false;
	}
}

bool_t Client::CValtanActionWorkbench::Apply_AnimationOccurrenceTiming(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const std::string& strClipOccurrenceId,
	const uint32_t iSourceStartMs,
	const uint32_t iPlayMs,
	std::string& strOutStatus)
{
	if (nullptr == m_pBalanceTool || nullptr == m_pAnimationTool)
	{
		strOutStatus =
			"Animation occurrence timing requires the typed Stage and native Animation owners.";
		return false;
	}
	if ("WAIT" == Stage.strSequenceRole)
	{
		strOutStatus =
			"WAIT Stage timing is read-only because WAIT must remain Animation NONE.";
		return false;
	}

	CBalanceTool::PATTERN_STAGE_EDIT Draft;
	if (!m_pBalanceTool->Get_ValtanStageDraft(
			Pattern.strPatternId, Stage.strStageId, Draft, strOutStatus))
	{
		return false;
	}
	if (!Draft.animationEditable)
	{
		strOutStatus =
			"This Stage does not admit typed Animation occurrence timing edits.";
		return false;
	}
	const std::size_t iMatchCount = static_cast<std::size_t>(std::count_if(
		Draft.animationSlots.begin(), Draft.animationSlots.end(),
		[&strClipOccurrenceId](const CBalanceTool::ANIMATION_SLOT_EDIT& Slot)
		{
			return Slot.clipOccurrenceId == strClipOccurrenceId;
		}));
	const auto Found = std::find_if(
		Draft.animationSlots.begin(), Draft.animationSlots.end(),
		[&strClipOccurrenceId](const CBalanceTool::ANIMATION_SLOT_EDIT& Slot)
		{
			return Slot.clipOccurrenceId == strClipOccurrenceId;
		});
	if (1u != iMatchCount || Draft.animationSlots.end() == Found)
	{
		strOutStatus =
			"Animation timing edit requires one exact stable clipOccurrenceId: " +
			strClipOccurrenceId + ".";
		return false;
	}
	if (Found->repeatUntilStageEnd && 0u != iPlayMs)
	{
		strOutStatus =
			"A LOOP_TO_STAGE_END occurrence must keep playMs=0; change the typed end policy in Details before making it finite.";
		return false;
	}
	if (Found->sourceStartMs == iSourceStartMs && Found->playMs == iPlayMs)
	{
		strOutStatus = "Animation occurrence timing is unchanged.";
		return true;
	}

	/* stable occurrence ID·clip·mapping·순서·재생률·반복 정책은 유지한다.
	   native 원본 구간과 clip을 지정한 Effect/Sound/Shake 종속 항목을 모두 승인한 뒤 공용 Balance draft를 바꾼다.
	   Stage 길이는 고정하며 EXACT playlist가 짧아진 경우 아래에서 명시적으로 HOLD_LAST_POSE로 바꾼다. */
	Found->sourceStartMs = iSourceStartMs;
	Found->playMs = iPlayMs;
    // 유한 playlist를 줄여도 저작 Server 시각은 유지한다.
    // 끝의 빈 EXACT 구간을 남기는 대신 마지막 자세를 유지한다.
    bool holdTail = false;
    if (Draft.animationEndPolicy == "EXACT")
    {
        uint64_t wall = 0u;
        for (const auto& slot : Draft.animationSlots)
        {
            if (slot.repeatUntilStageEnd || !std::isfinite(slot.playRate) || slot.playRate <= 0.0)
            { strOutStatus = "Finite source trim lost its exact playlist clock."; return false; }
            uint32_t play = slot.playMs;
            if (!play)
            {
                uint32_t native = 0u;
                if (!m_pAnimationTool->Resolve_ValtanCompositionNativeClipDurationMs(slot.clip, native, strOutStatus) ||
                    slot.sourceStartMs >= native) return false;
                play = native - slot.sourceStartMs;
            }
            wall += static_cast<uint64_t>(std::llround(static_cast<double>(play) / slot.playRate));
        }
        holdTail = wall < Draft.durationMs;
        if (holdTail) Draft.animationEndPolicy = "HOLD_LAST_POSE";
    }
	if (!SetValtanStageDraftWithSoundDependencyAdmission(
			m_pAnimationTool, m_pBalanceTool,
			m_bPatternShakesReady ? &m_PatternShakes : nullptr,
			Pattern, Stage, Draft, strOutStatus))
	{
		return false;
	}
	strOutStatus = "Staged Animation source trim for " + strClipOccurrenceId +
        ". Following clips ripple inside the unchanged Server Stage clock. Use Save to commit.";
    if (holdTail) strOutStatus += " End Policy changed to HOLD_LAST_POSE for the remaining Stage tail.";
    Refresh_PatternLocalPreviewAfterMutation(&Pattern, strOutStatus);
	return true;
}

uint32_t Client::CValtanActionWorkbench::Resolve_ClipSourceToStageMs(
	const VALTAN_STAGE_VIEW& Stage,
	const std::string& strClipOccurrenceId,
	const uint32_t iSourceMs) const
{
	uint64_t iWallCursorMs = 0u;
	for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip : Stage.ClipOccurrences)
	{
		if (Clip.strClipOccurrenceId == strClipOccurrenceId)
		{
			const uint32_t iLocalSourceMs = iSourceMs > Clip.iSourceStartMs ?
				iSourceMs - Clip.iSourceStartMs : 0u;
			const double fRate = Clip.fPlayRate > 0.f ? Clip.fPlayRate : 1.0;
			const uint64_t iLocalWallMs = static_cast<uint64_t>(std::llround(
				static_cast<double>(iLocalSourceMs) / fRate));
			return static_cast<uint32_t>((std::min)(
				iWallCursorMs + iLocalWallMs,
				static_cast<uint64_t>(Stage.iDurationMs)));
		}
		iWallCursorMs += Clip.iAuthoringWallMs;
	}
	return 0u;
}

void Client::CValtanActionWorkbench::Invalidate_TimelineCache()
{
	m_strTimelineCachePatternId.clear();
	m_eTimelineCachePreviewPath = VALTAN_PATTERN_PREVIEW_PATH::END;
	m_iTimelineCacheDraftGeneration = ~std::uint64_t{ 0u };
	m_iTimelineCacheSoundGeneration = ~std::uint64_t{ 0u };
	m_iTimelineCacheEffectV2Revision = ~std::uint64_t{ 0u };
	m_iTimelineCacheEffectV1Revision = ~std::uint64_t{ 0u };
}

void Client::CValtanActionWorkbench::Invalidate_EffectivePatternCache()
{
	m_bCompositionResourceDraftReady = false;
	m_EffectivePatternCache = {};
	m_strEffectivePatternCachePatternId.clear();
	m_strEffectivePatternCacheCanonicalRevision.clear();
	m_iEffectivePatternCacheDraftGeneration = ~std::uint64_t{ 0u };
	m_bEffectivePatternCacheReady = false;
}

// pane들이 같은 Pattern을 그릴 때 owner generation/revision을 cache key와 비교한다.
// 일치하면 표시 행을 재사용하고 달라지면 Build_Timeline로 projection을 다시 만든다. cache는 원본 draft를 대체하지 않는다.
void Client::CValtanActionWorkbench::Ensure_TimelineCache(
	const VALTAN_PATTERN_VIEW* const pPattern)
{
	if (nullptr == pPattern)
	{
		m_TimelineItems.clear();
		Invalidate_TimelineCache();
		m_iTimelineDurationMs = 0u;
		m_iPlayheadMs = 0u;
		return;
	}
	const std::uint64_t iDraftGeneration = nullptr == m_pBalanceTool ? 0u :
		m_pBalanceTool->Get_ValtanDraftGeneration();
	const std::uint64_t iSoundGeneration = nullptr == m_pAnimationTool ? 0u :
		m_pAnimationTool->Get_ValtanCompositionPatternSoundDraftGeneration();
	const std::uint64_t iEffectV2Revision =
		CEffectV2Catalog::Get().Get_Revision();
	if (m_strTimelineCachePatternId == pPattern->strPatternId &&
		m_eTimelineCachePreviewPath == m_ePreviewPath &&
		m_iTimelineCacheDraftGeneration == iDraftGeneration &&
		m_iTimelineCacheSoundGeneration == iSoundGeneration &&
		m_iTimelineCacheEffectV2Revision == iEffectV2Revision &&
		m_iTimelineCacheEffectV1Revision == CEffectCatalog::Get_RuntimeRevision())
	{
		m_iPlayheadMs = (std::min)(m_iPlayheadMs, m_iTimelineDurationMs);
		return;
	}
	Build_Timeline(*pPattern);
	m_iPlayheadMs = (std::min)(m_iPlayheadMs, m_iTimelineDurationMs);
}

// 선택한 preview Stage 경로의 누적 ms 위에 Animation·Effect·Sound·판정 행을 배치한다.
// 각 행에는 원본을 다시 찾을 ID와 수정 가능 여부를 남긴다. pixel 폭·겹침 subrow는 표시용이며 Server 시각을 바꾸지 않는다.
void Client::CValtanActionWorkbench::Build_Timeline(
	const VALTAN_PATTERN_VIEW& Pattern)
{
	m_TimelineItems.clear();
	bool_t bPatternSoundDirty = false;
	std::string PatternSoundStatus;
	const VALTAN_PATTERN_SOUND_CUE_DOCUMENT* const pPatternSounds =
		VALTAN_VIEW_ADMISSION::ADMITTED != m_eAdmission ||
		nullptr == m_pAnimationTool ? nullptr :
		m_pAnimationTool->Get_ValtanCompositionPatternSoundDraft(
			bPatternSoundDirty, PatternSoundStatus);
	CSoundCueCatalog::EVENT_VARIANTS SoundEvents;
	std::unordered_map<std::string, uint32_t> SoundDurations;
	std::unordered_map<std::string, f32_t> SoundClipDurations;
	std::string SoundCatalogStatus;
	if (pPatternSounds)
		(void)CSoundCueCatalog::Load_ClassSnapshot("Valtan", SoundEvents, SoundCatalogStatus);
	const auto ResolveSoundDuration = [&SoundDurations](const std::string& assetId) {
		const auto [entry, inserted] = SoundDurations.try_emplace(assetId, 0u);
		if (inserted)
		{
			const auto path = CRuntimeAssetRoot::Resolve(assetId);
			if (!path.empty())
				(void)CGameInstance::Get().Get_SoundDurationMs(path.wstring(), entry->second);
		}
		return entry->second;
	};
	const std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> pEffectV2Snapshot =
		CEffectV2Catalog::Get().Get_Snapshot();
	std::vector<const VALTAN_STAGE_VIEW*> PreviewStages;
	std::string PreviewPathStatus;
	if (!CValtanPatternTree::Build_PreviewStagePath(
			Pattern, m_ePreviewPath, PreviewStages, PreviewPathStatus))
	{
		if (VALTAN_PATTERN_PREVIEW_PATH::NORMAL == m_ePreviewPath ||
			!CValtanPatternTree::Build_PreviewStagePath(
				Pattern, VALTAN_PATTERN_PREVIEW_PATH::NORMAL,
				PreviewStages, PreviewPathStatus))
		{
			m_iTimelineDurationMs = 0u;
			m_strTimelineCachePatternId = Pattern.strPatternId;
			m_eTimelineCachePreviewPath = m_ePreviewPath;
			m_iTimelineCacheDraftGeneration = nullptr == m_pBalanceTool ? 0u :
				m_pBalanceTool->Get_ValtanDraftGeneration();
			m_iTimelineCacheSoundGeneration = nullptr == m_pAnimationTool ? 0u :
				m_pAnimationTool->Get_ValtanCompositionPatternSoundDraftGeneration();
			m_iTimelineCacheEffectV2Revision =
				CEffectV2Catalog::Get().Get_Revision();
			m_strStatus = "Sequencer branch rejected: " + PreviewPathStatus;
			return;
		}
		m_ePreviewPath = VALTAN_PATTERN_PREVIEW_PATH::NORMAL;
		m_iPreviewDraftGeneration = 0u;
		m_strStatus =
			"Selected branch is unavailable for this Pattern; Sequencer returned to Normal / Timeout.";
	}
	uint64_t iStageBaseMs = 0u;
	for (const VALTAN_STAGE_VIEW* const pStage : PreviewStages)
	{
		if (nullptr == pStage)
			continue;
		const VALTAN_STAGE_VIEW& Stage = *pStage;
		uint32_t iStageDurationMs = Stage.iDurationMs;
		CBalanceTool::PATTERN_STAGE_EDIT StageDraft;
		bool_t bHasStageDraft = false;
		if (VALTAN_VIEW_ADMISSION::ADMITTED == m_eAdmission &&
			nullptr != m_pBalanceTool)
		{
			std::string DraftStatus;
			bHasStageDraft = m_pBalanceTool->Get_ValtanStageDraft(
				Pattern.strPatternId, Stage.strStageId, StageDraft, DraftStatus);
			if (bHasStageDraft)
			{
				iStageDurationMs = StageDraft.durationMs;
			}
		}
		const auto ResolveSourceToStageMs =
			[&Stage, &StageDraft, bHasStageDraft, iStageDurationMs, this](
				const std::string& strClipOccurrenceId,
				const uint32_t iSourceMs)
		{
			if (!bHasStageDraft)
				return Resolve_ClipSourceToStageMs(
					Stage, strClipOccurrenceId, iSourceMs);
			uint64_t iWallCursorMs = 0u;
			for (const CBalanceTool::ANIMATION_SLOT_EDIT& Slot :
				StageDraft.animationSlots)
			{
				if (Slot.clipOccurrenceId == strClipOccurrenceId)
				{
					const uint32_t iLocalSourceMs = iSourceMs > Slot.sourceStartMs ?
						iSourceMs - Slot.sourceStartMs : 0u;
					const uint64_t iLocalWallMs = static_cast<uint64_t>(
						std::llround(static_cast<double>(iLocalSourceMs) /
							Slot.playRate));
					return static_cast<uint32_t>((std::min)(
						iWallCursorMs + iLocalWallMs,
						static_cast<uint64_t>(iStageDurationMs)));
				}
				const uint32_t iSlotWallMs = 0u == Slot.playMs ?
					(iStageDurationMs > iWallCursorMs ?
						static_cast<uint32_t>(iStageDurationMs - iWallCursorMs) : 0u) :
					static_cast<uint32_t>(std::llround(
						static_cast<double>(Slot.playMs) / Slot.playRate));
				iWallCursorMs += iSlotWallMs;
			}
			return 0u;
		};
		const uint32_t iStageStartMs = static_cast<uint32_t>(
			SaturatingU32(iStageBaseMs));
		const uint32_t iStageEndMs = static_cast<uint32_t>(SaturatingU32(
			iStageBaseMs + static_cast<uint64_t>(iStageDurationMs)));
		m_TimelineItems.push_back({
			DETAIL_OWNER::GAMEPLAY_STAGE, TIMELINE_LANE::STAGE, Pattern.strPatternId,
			Stage.strStageId, Stage.strStageId, {},
			Stage.strStageId + " | " + StageSequenceRoleLabel(Stage),
			iStageStartMs, iStageEndMs,
			bHasStageDraft && StageDraft.durationEditable });

		const std::string strEffectiveHitShape = bHasStageDraft ?
			StageDraft.hitShape : Stage.strHitShape;
		if (!strEffectiveHitShape.empty() && "NONE" != strEffectiveHitShape)
		{
			const bool_t bHasHitActivation = bHasStageDraft ?
				StageDraft.hasHitActivation : Stage.bHasHitActivation;
			const uint32_t iHitActivationStartMs = bHasStageDraft ?
				StageDraft.hitActivationStartMs : Stage.iHitActivationStartMs;
			const uint32_t iHitActivationLifetimeMs = bHasStageDraft ?
				StageDraft.hitActivationLifetimeMs : Stage.iHitActivationLifetimeMs;
			const uint32_t iHitDelayMs = bHasStageDraft ?
				StageDraft.hitDelayMs : Stage.iHitDelayMs;
			const uint32_t iHitIntervalMs = bHasStageDraft ?
				StageDraft.hitIntervalMs : Stage.iHitIntervalMs;
			const uint32_t iHitCount = bHasStageDraft ?
				StageDraft.hitCount : Stage.iHitCount;
			const std::vector<uint32_t>& HitOffsets = bHasStageDraft ?
				StageDraft.hitOffsetsMs : Stage.HitOffsetsMs;
			uint32_t iFirstHitMs = iHitDelayMs;
			uint32_t iLastHitMs = iHitDelayMs;
			if (bHasHitActivation)
			{
				iFirstHitMs = iHitActivationStartMs;
				iLastHitMs = static_cast<uint32_t>((std::min)(
					static_cast<uint64_t>(iStageDurationMs),
					static_cast<uint64_t>(iHitActivationStartMs) +
						iHitActivationLifetimeMs));
			}
			else if (!HitOffsets.empty())
			{
				iFirstHitMs = *std::min_element(
					HitOffsets.begin(), HitOffsets.end());
				iLastHitMs = *std::max_element(
					HitOffsets.begin(), HitOffsets.end());
			}
			else if (iHitCount > 1u)
			{
				iLastHitMs += iHitIntervalMs * (iHitCount - 1u);
			}
			m_TimelineItems.push_back({
				DETAIL_OWNER::GAMEPLAY_STAGE, TIMELINE_LANE::COLLIDER,
				Pattern.strPatternId, Stage.strStageId,
				Stage.strStageId + "/collider", {},
				std::string("CAPTURE" == Stage.strPlayerResponse ?
					"GRAB " : "DAMAGE ") +
					strEffectiveHitShape + " | " +
					(bHasHitActivation ?
						"active " + std::to_string(iHitActivationLifetimeMs) +
							" ms" :
						std::to_string(iHitCount) + " hit(s)"),
				iStageStartMs + (std::min)(iFirstHitMs, iStageDurationMs),
				iStageStartMs + (std::min)(
					iLastHitMs, iStageDurationMs),
				bHasStageDraft && StageDraft.colliderTuneAdmitted });
		}
		/* 플레이어와 관계된 나머지 Server 판정도 같은 lane에 읽기 전용으로 표시한다.
		   Stage 내내 열린 counter hurt proxy와 combat object의 donut·도끼·발사체가 대상이다.
		   combat object의 타격 시각은 해당 pipeline이 별도로 소유한다. */
		if (Stage.CounterProxy.has_value())
		{
			const VALTAN_COUNTER_PROXY_VIEW& Proxy = *Stage.CounterProxy;
			char ProxyLabel[160]{};
			std::snprintf(ProxyLabel, sizeof(ProxyLabel),
				"COUNTER proxy | %s r %.2f m%s",
				Proxy.strKind.c_str(), Proxy.fRadiusM,
				Proxy.fArcDegrees > 0.f ? " (arc)" : "");
			m_TimelineItems.push_back({
				DETAIL_OWNER::GAMEPLAY_STAGE, TIMELINE_LANE::COLLIDER,
				Pattern.strPatternId, Stage.strStageId,
				Stage.strStageId + "/counter-proxy", {},
				ProxyLabel, iStageStartMs, iStageEndMs, false });
		}
		for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Object :
			Stage.CombatObjectEffects)
		{
			if (Object.Hits.empty() && Object.HitOffsetsMs.empty())
				continue;
			/* 생성 wave마다 첫 offset+간격을 나타내는 부모 행 하나를 만들고, object에 시간별 타격이 있으면
			   그 wave 안의 각 hit offset도 행으로 만든다. 각 행은 archetype을 stable ID로 사용해 같은 Combat Object Details 소유자를 찾는다. */
			const uint32_t iWaveCount =
				(std::max)(Object.iSpawnScheduleCount, 1u);
			constexpr uint32_t MAX_TIMELINE_WAVES = 8u;
			const std::string strHitCount = std::to_string((std::max)(
				Object.Hits.size(), Object.HitOffsetsMs.size())) + " hit(s)";
			for (uint32_t iWave = 0u;
				iWave < iWaveCount && iWave < MAX_TIMELINE_WAVES; ++iWave)
			{
				const uint32_t iWaveStartMs = static_cast<uint32_t>(
					SaturatingU32(static_cast<uint64_t>(iStageStartMs) +
						Object.iFirstSpawnOffsetMs +
						static_cast<uint64_t>(Object.iSpawnIntervalMs) * iWave));
				const uint32_t iWaveEndMs = static_cast<uint32_t>(
					SaturatingU32(static_cast<uint64_t>(iWaveStartMs) +
						Object.iLifetimeMs));
				const std::string strWave = iWaveCount > 1u ?
					" wave " + std::to_string(iWave + 1u) + "/" +
						std::to_string(iWaveCount) : std::string();
				m_TimelineItems.push_back({
					DETAIL_OWNER::COMBAT_OBJECT, TIMELINE_LANE::COLLIDER,
					Pattern.strPatternId, Stage.strStageId,
					Object.strCombatObjectArchetypeId, {},
					"OBJECT " + Object.strCombatObjectArchetypeId + strWave +
						" | " + strHitCount,
					iWaveStartMs, iWaveEndMs, false });
				for (const uint32_t iHitOffsetMs : Object.HitOffsetsMs)
				{
					const uint32_t iHitMs = static_cast<uint32_t>(
						SaturatingU32(static_cast<uint64_t>(iWaveStartMs) +
							iHitOffsetMs));
					m_TimelineItems.push_back({
						DETAIL_OWNER::COMBAT_OBJECT, TIMELINE_LANE::COLLIDER,
						Pattern.strPatternId, Stage.strStageId,
						Object.strCombatObjectArchetypeId, {},
						"HIT " + Object.strCombatObjectArchetypeId + strWave +
							" @+" + std::to_string(iHitOffsetMs) + " ms",
						iHitMs, iHitMs, false });
				}
			}
		}

		for (const VALTAN_STAGE_BRANCH_VIEW& Branch : Stage.Branches)
		{
			const std::string strTarget = BuildBranchTargetLabel(Branch);
			const bool_t bCounterBranch = "COUNTER_HIT" == Branch.strOutcome;
			m_TimelineItems.push_back({
				DETAIL_OWNER::GAMEPLAY_STAGE, TIMELINE_LANE::LOGIC,
				Pattern.strPatternId, Stage.strStageId,
				BuildBranchStableId(Stage.strStageId, Branch.strOutcome),
				{}, Branch.strOutcome + " -> " + strTarget,
				iStageStartMs, iStageEndMs,
				bHasStageDraft && bCounterBranch &&
					"WINDUP" == StageDraft.stageKind });
		}
		const std::vector<VALTAN_STAGE_ACTION_VIEW>& EffectiveActions =
			bHasStageDraft ? StageDraft.actions : Stage.Actions;
		for (const VALTAN_STAGE_ACTION_VIEW& Action : EffectiveActions)
		{
			const bool spawn = Action.strKind.find("SPAWN_COMBAT_OBJECT") == 0u;
			const bool world = Action.strKind == "TRIGGER_WORLD_EVENT_SET";
			const auto lane = spawn ? TIMELINE_LANE::SUMMON : (world ? TIMELINE_LANE::WORLD : TIMELINE_LANE::LOGIC);
			const std::string id = Stage.strStageId + "/action/" + Action.strKind + "/" + Action.strTargetId + "/" + Action.strTrigger;
			const uint32_t start = Action.strTrigger == "EXIT" ? iStageEndMs : iStageStartMs;
			m_TimelineItems.push_back({DETAIL_OWNER::GAMEPLAY_STAGE, lane,
				Pattern.strPatternId, Stage.strStageId, id, Action.strTargetId,
				Action.strKind + " | " + Action.strTargetId + " [" + Action.strTrigger + "]",
				start, Action.strTrigger == "EXIT" ? start : iStageEndMs,
				bHasStageDraft && !spawn &&
				!(Action.strKind == "SET_BOSS_FLAG" && (Action.strTargetId == "boss.flag.counterable" || Action.strTargetId == "boss.flag.groggy"))});
		}
		if ((bHasStageDraft && StageDraft.portalRushMotionEditable) ||
			Stage.Motion.has_value())
		{
			const std::string strMotionKind = bHasStageDraft ?
				StageDraft.motionKind : Stage.Motion->strKind;
			if ("PORTAL_TARGET_RUSH" == strMotionKind)
			{
				const uint32_t iRetargetDelayMs = bHasStageDraft ?
					StageDraft.portalRetargetDelayMs :
					Stage.Motion->iRetargetDelayMs;
				const double fSpeedMps = bHasStageDraft ?
					StageDraft.portalSpeedMps : Stage.Motion->fSpeedMps;
				const double fDistanceM = bHasStageDraft ?
					StageDraft.portalDistanceM : Stage.Motion->fDistance;
				const uint32_t iTravelMs =
					std::isfinite(fSpeedMps) && std::isfinite(fDistanceM) &&
					fSpeedMps > 0.0 && fDistanceM > 0.0 ?
					static_cast<uint32_t>(std::ceil(
						fDistanceM / fSpeedMps * 1000.0)) : 0u;
				const uint32_t iDelayEndMs = (std::min)(
					iRetargetDelayMs, iStageDurationMs);
				const uint32_t iTravelEndMs = static_cast<uint32_t>((std::min)(
					static_cast<uint64_t>(iStageDurationMs),
					static_cast<uint64_t>(iDelayEndMs) + iTravelMs));
				const uint32_t iTrailingGapMs =
					iStageDurationMs - iTravelEndMs;

				m_TimelineItems.push_back({
					DETAIL_OWNER::GAMEPLAY_STAGE, TIMELINE_LANE::LOGIC,
					Pattern.strPatternId, Stage.strStageId,
					Stage.strStageId + "/motion/delay", {},
					"Target Delay | " + std::to_string(iDelayEndMs) + " ms",
					iStageStartMs, iStageStartMs + iDelayEndMs, false });
				std::ostringstream RushLabel;
				RushLabel << "Rush | " << iTravelMs << " ms / " <<
					std::fixed << std::setprecision(1) << fDistanceM << " m";
				m_TimelineItems.push_back({
					DETAIL_OWNER::GAMEPLAY_STAGE, TIMELINE_LANE::LOGIC,
					Pattern.strPatternId, Stage.strStageId,
					Stage.strStageId + "/motion/travel", {}, RushLabel.str(),
					iStageStartMs + iDelayEndMs,
					iStageStartMs + iTravelEndMs, false });
				m_TimelineItems.push_back({
					DETAIL_OWNER::GAMEPLAY_STAGE, TIMELINE_LANE::LOGIC,
					Pattern.strPatternId, Stage.strStageId,
					Stage.strStageId + "/motion/gap", {},
					"Gap | " + std::to_string(iTrailingGapMs) + " ms",
					iStageStartMs + iTravelEndMs, iStageEndMs, false });
			}
			else
			{
				m_TimelineItems.push_back({
					DETAIL_OWNER::GAMEPLAY_STAGE, TIMELINE_LANE::LOGIC,
					Pattern.strPatternId, Stage.strStageId,
					Stage.strStageId + "/motion/" + strMotionKind, {},
					"TO_ARENA_CENTER" == strMotionKind ?
						"Move to arena center | " + std::to_string(iStageDurationMs) + " ms" :
						"Motion | " + strMotionKind,
					iStageStartMs, iStageEndMs,
					bHasStageDraft && StageDraft.portalRushMotionEditable });
			}
		}

		const std::string& strAimTargetPolicy = bHasStageDraft ?
			StageDraft.aimTargetPolicy : Stage.strAimTargetPolicy;
		if (!strAimTargetPolicy.empty())
		{
			m_TimelineItems.push_back({
				DETAIL_OWNER::GAMEPLAY_STAGE, TIMELINE_LANE::LOGIC,
				Pattern.strPatternId, Stage.strStageId,
				Stage.strStageId + "/aim", {}, "Aim | " + strAimTargetPolicy,
				iStageStartMs, iStageEndMs, false });
		}

		uint64_t iAnimationCursorMs = iStageBaseMs;
		if (bHasStageDraft)
		{
			for (const CBalanceTool::ANIMATION_SLOT_EDIT& Slot :
				StageDraft.animationSlots)
			{
				const uint32_t iStartMs = static_cast<uint32_t>(
					SaturatingU32(iAnimationCursorMs));
				const uint32_t iWallMs = 0u == Slot.playMs ?
					(iStageDurationMs > iAnimationCursorMs - iStageBaseMs ?
						static_cast<uint32_t>(iStageDurationMs -
							(iAnimationCursorMs - iStageBaseMs)) : 0u) :
					static_cast<uint32_t>(std::llround(
						static_cast<double>(Slot.playMs) / Slot.playRate));
				iAnimationCursorMs += iWallMs;
				m_TimelineItems.push_back({
					DETAIL_OWNER::ANIMATION, TIMELINE_LANE::ANIMATION, Pattern.strPatternId,
					Stage.strStageId, Slot.clipOccurrenceId, Slot.clip,
					Slot.repeatUntilStageEnd ?
						Slot.clip + " [loop to Stage end]" : Slot.clip,
					iStartMs,
					static_cast<uint32_t>(SaturatingU32(iAnimationCursorMs)),
					StageDraft.animationEditable });
				m_TimelineItems.back().bLoopsToStageEnd = Slot.repeatUntilStageEnd;
			}
		}
		else
		{
			for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip : Stage.ClipOccurrences)
			{
				const uint32_t iStartMs = static_cast<uint32_t>(
					SaturatingU32(iAnimationCursorMs));
				iAnimationCursorMs += Clip.iAuthoringWallMs;
				m_TimelineItems.push_back({
					DETAIL_OWNER::ANIMATION, TIMELINE_LANE::ANIMATION, Pattern.strPatternId,
					Stage.strStageId, Clip.strClipOccurrenceId, Clip.strClipName,
					Clip.strClipName, iStartMs,
					static_cast<uint32_t>(SaturatingU32(iAnimationCursorMs)), false });
			}
		}

		for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Cue : Stage.ProductCues)
		{
			const uint32_t iLocalStartMs = Cue.bUsesStageClock ?
				Cue.iStageOffsetMs : ResolveSourceToStageMs(
					Cue.strClipOccurrenceId, Cue.iSourceStartMs);
			uint32_t iLocalEndMs = iLocalStartMs;
			// 자연 수명 cue는 순간 marker가 아닌 resource 시각을 가진다.
			// revision으로 무효화된 timeline을 다시 만들 때만 문서를 읽는다.
			if (const auto document = CEffectCatalog::Find(Cue.strEffectAssetId))
			{
				double rate = 1.0;
				for (const auto& clip : Stage.ClipOccurrences)
					if (clip.strClipOccurrenceId == Cue.strClipOccurrenceId && clip.fPlayRate > 0.f)
						rate = clip.fPlayRate;
				if (bHasStageDraft)
					for (const auto& slot : StageDraft.animationSlots)
						if (slot.clipOccurrenceId == Cue.strClipOccurrenceId && slot.playRate > 0.0)
							rate = slot.playRate;
				const uint32_t span = ResolveEffectDocumentSpanMs(*document);
				const uint32_t sourceSeek = ResolveEffectPlaybackOffsetMs(Cue);
				const uint64_t remaining = static_cast<uint64_t>(std::ceil(
					static_cast<double>(span > sourceSeek ? span - sourceSeek : 0u) / rate));
				iLocalEndMs = static_cast<uint32_t>(SaturatingU32(uint64_t(iLocalStartMs) + remaining));
			}
			if (Cue.bHasSourceEnd)
			{
				double rate = 1.0;
				for (const auto& clip : Stage.ClipOccurrences)
					if (clip.strClipOccurrenceId == Cue.strClipOccurrenceId && clip.fPlayRate > 0.f)
						rate = clip.fPlayRate;
				if (bHasStageDraft)
					for (const auto& slot : StageDraft.animationSlots)
						if (slot.clipOccurrenceId == Cue.strClipOccurrenceId && slot.playRate > 0.0)
							rate = slot.playRate;
				// 명시적 ONCE의 잔여 수명은 소유 Stage가 끝난 뒤에도 resource 시각을 유지한다.
				const auto span = Cue.iSourceEndMs > Cue.iSourceStartMs ?
					static_cast<uint64_t>(std::ceil((Cue.iSourceEndMs - Cue.iSourceStartMs) / rate)) : 0u;
				iLocalEndMs = static_cast<uint32_t>(SaturatingU32(uint64_t(iLocalStartMs) + span));
			}
			if ("each_loop" == Cue.strRepeatPolicy)
				iLocalEndMs = (std::max)(iLocalEndMs, iStageDurationMs);
			m_TimelineItems.push_back({
				DETAIL_OWNER::EFFECT, TIMELINE_LANE::EFFECT, Pattern.strPatternId,
				Stage.strStageId, Cue.strOccurrenceId, Cue.strEffectAssetId,
				Cue.strEffectAssetId + " [" + Cue.strStopPolicy + "/" +
					Cue.strRepeatPolicy + "]",
				iStageStartMs + (std::min)(iLocalStartMs, iStageDurationMs),
				static_cast<uint32_t>(SaturatingU32(uint64_t(iStageStartMs) + iLocalEndMs)),
				true });
		}
		for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Object :
			Stage.CombatObjectEffects)
		{
			const uint32_t iObjectStartMs = static_cast<uint32_t>(
				SaturatingU32(static_cast<uint64_t>(iStageStartMs) +
					Object.iFirstSpawnOffsetMs));
			const uint32_t iObjectEndMs = static_cast<uint32_t>(
				SaturatingU32(static_cast<uint64_t>(iObjectStartMs) +
					Object.iLifetimeMs));
			const std::string& strTimelineResource =
				Object.strEffectV2GroupId.empty() ? Object.strEffectAssetId :
					Object.strEffectV2GroupId;
			std::string strSpawnSummary = " x" +
				std::to_string(Object.iSpawnValue);
			if (!Object.strVolleyPolicy.empty())
			{
				strSpawnSummary += " | waves " +
					std::to_string(Object.iSpawnScheduleCount) + " every " +
					std::to_string(Object.iSpawnIntervalMs) + "ms";
				if (0u != Object.iArenaRandomCount)
				{
					strSpawnSummary += " | arena random " +
						std::to_string(Object.iArenaRandomCount);
				}
			}
			m_TimelineItems.push_back({
				DETAIL_OWNER::COMBAT_OBJECT, TIMELINE_LANE::SUMMON, Pattern.strPatternId,
				Stage.strStageId, Object.strCombatObjectArchetypeId,
				strTimelineResource,
				"World Object" + strSpawnSummary +
					" @+" + std::to_string(Object.iFirstSpawnOffsetMs) +
					"ms | " +
					(Object.strEffectV2GroupId.empty() ? "V1 " : "V2 Group ") +
					strTimelineResource,
				iObjectStartMs,
				iObjectEndMs,
				bHasStageDraft });
		}
		if (nullptr != pEffectV2Snapshot && pEffectV2Snapshot->Is_Ready())
		{
            const auto AppendEffectV2TimelineRow =
                [this, &Pattern, &Stage, iStageStartMs](
                    const EFFECT_V2_BINDING& Binding,
                    const uint32_t iLocalStartMs,
                    const uint32_t iLocalEndMs,
                    const std::string_view strClipOccurrenceId,
                    const f32_t fClipPlayRate,
                    const bool_t bRepeatUntilStageEnd,
                    const EFFECT_V2_TIMELINE_SPAN& Span)
            {
                const bool_t bGroup = !Binding.strGroupId.empty();
                const std::string& strResourceId = bGroup ? Binding.strGroupId : Binding.strEffectId;
                const std::string strStableId = BuildEffectV2BindingStableId(Binding, strClipOccurrenceId);
                std::string strLabel = std::string("V2 ") +
                    (bGroup ? "Group | " : "Leaf | ") + strResourceId;
                if (EFFECT_V2_CLOCK_BASIS::CLIP_OCCURRENCE == Binding.eClockBasis)
                {
                    strLabel += " | Clip " + std::string(strClipOccurrenceId);
                    if (bRepeatUntilStageEnd) strLabel += " [each loop]";
                }
                if (!Span.bResolved) strLabel += " [duration unresolved: shown to Stage end]";
                else if (Span.bUnbounded) strLabel += " [unbounded: shown to Stage end]";
                m_TimelineItems.push_back({
                    DETAIL_OWNER::EFFECT, TIMELINE_LANE::EFFECT,
                    Pattern.strPatternId, Stage.strStageId, strStableId,
                    strResourceId, std::move(strLabel),
                    static_cast<uint32_t>(SaturatingU32(uint64_t(iStageStartMs) + iLocalStartMs)),
                    static_cast<uint32_t>(SaturatingU32(uint64_t(iStageStartMs) + iLocalEndMs)), false, 0u,
                    bGroup ? EFFECT_V2_GROUP_MINIMUM_WIDTH_PX : EFFECT_V2_LEAF_MINIMUM_WIDTH_PX,
                    true, bGroup, Binding.iStartMs,
                    std::string(strClipOccurrenceId), fClipPlayRate });
            };

            for (const EFFECT_V2_BINDING& Binding : pEffectV2Snapshot->Get_BossValtanBindings())
            {
                if (Binding.strPatternId != Pattern.strPatternId ||
                    Binding.strStageId != Stage.strStageId || Binding.strActionId != Stage.strActionId)
                    continue;
                if (EFFECT_V2_CLOCK_BASIS::STAGE == Binding.eClockBasis)
                {
                    if (Binding.iStartMs >= iStageDurationMs) continue;
                    const auto span = ResolveEffectV2ResourceSpanMs(*pEffectV2Snapshot, Binding);
                    const uint32_t end = ResolveEffectV2TimelineEndMs(span, Binding.iStartMs,
                        1.0, iStageDurationMs, Binding.eStopPolicy);
                    AppendEffectV2TimelineRow(Binding, Binding.iStartMs, end, {}, 1.f, false, span);
                    continue;
                }
                if (EFFECT_V2_CLOCK_BASIS::CLIP_OCCURRENCE != Binding.eClockBasis ||
                    Binding.strClipOccurrenceId.empty()) continue;
                const auto ProjectClipBinding =
                    [&Binding, &AppendEffectV2TimelineRow, &pEffectV2Snapshot, iStageDurationMs](
                        const std::string& strClipName, const std::string& strClipOccurrenceId,
                        const uint32_t iSourceStartMs, const uint32_t iSourcePlayMs,
                        const double fPlayRate, const bool_t bRepeatUntilStageEnd,
                        const uint32_t iClipWallStartMs, const uint32_t iClipWallEndMs)
                {
                    (void)strClipName;
                    if (Binding.strClipOccurrenceId != strClipOccurrenceId ||
                        Binding.iStartMs < iSourceStartMs || !std::isfinite(fPlayRate) || fPlayRate <= 0.0)
                        return;
                    const uint64_t sourceEnd = uint64_t(iSourceStartMs) + iSourcePlayMs;
                    if (iSourcePlayMs && Binding.iStartMs >= sourceEnd) return;
                    const double start = double(iClipWallStartMs) +
                        double(Binding.iStartMs - iSourceStartMs) / fPlayRate;
                    if (start >= iClipWallEndMs || start >= iStageDurationMs) return;
                    const bool repeat = bRepeatUntilStageEnd &&
                        EFFECT_V2_REPEAT_POLICY::EACH_LOOP == Binding.eRepeatPolicy;
                    // clip 종료는 child의 Kill/Deactivate 방식을 따른다.
                    // Stage 끝에서는 STAGE_END와 CLIP_OCCURRENCE_END 모두 추가로 종료한다.
                    const double period = iSourcePlayMs ? double(iSourcePlayMs) / fPlayRate : 0.0;
                    const double firstClipEnd = repeat && period > 0.0 ?
                        (std::min)(double(iClipWallEndMs), double(iClipWallStartMs) + period) :
                        double(iClipWallEndMs);
                    const double clipStop = Binding.eStopPolicy == EFFECT_V2_STOP_POLICY::CLIP_OCCURRENCE_END ?
                        (std::max)(0.0, (firstClipEnd - start) * fPlayRate) : -1.0;
                    auto span = ResolveEffectV2ResourceSpanMs(*pEffectV2Snapshot, Binding, clipStop);
                    double lastStart = start;
                    if (repeat && period > 0.0)
                    {
                        const double epochs = (std::max)(0.0,
                            std::ceil((double(iClipWallEndMs) - start) / period) - 1.0);
                        lastStart += epochs * period;
                        // 마지막 반복 구간은 Stage 끝에서 잘릴 수 있다.
                        if (Binding.eStopPolicy == EFFECT_V2_STOP_POLICY::CLIP_OCCURRENCE_END)
                            span = ResolveEffectV2ResourceSpanMs(*pEffectV2Snapshot, Binding,
                                (std::max)(0.0, (double(iClipWallEndMs) - lastStart) * fPlayRate));
                    }
                    else if (repeat)
                        span.bResolved = false; // 이 snapshot에는 native 반복 주기가 없다.
                    const uint32_t end = ResolveEffectV2TimelineEndMs(span, lastStart,
                        fPlayRate, iStageDurationMs, Binding.eStopPolicy);
                    AppendEffectV2TimelineRow(Binding, static_cast<uint32_t>(std::llround(start)),
                        end, strClipOccurrenceId, static_cast<f32_t>(fPlayRate), repeat, span);
                };

				uint64_t iClipWallCursorMs = 0u;
				if (bHasStageDraft)
				{
					for (const CBalanceTool::ANIMATION_SLOT_EDIT& Slot :
						StageDraft.animationSlots)
					{
						if (!std::isfinite(Slot.playRate) || Slot.playRate <= 0.0)
							continue;
						const uint64_t iRemainingStageMs = iStageDurationMs >
							iClipWallCursorMs ? iStageDurationMs - iClipWallCursorMs : 0u;
						const uint64_t iClipWallMs =
							Slot.repeatUntilStageEnd || 0u == Slot.playMs ?
							iRemainingStageMs : static_cast<uint64_t>(std::llround(
								static_cast<double>(Slot.playMs) / Slot.playRate));
						const uint32_t iClipWallStartMs = static_cast<uint32_t>(
							(std::min)(iClipWallCursorMs,
								static_cast<uint64_t>(iStageDurationMs)));
						const uint32_t iClipWallEndMs = static_cast<uint32_t>(
							(std::min)(iClipWallCursorMs + iClipWallMs,
								static_cast<uint64_t>(iStageDurationMs)));
						ProjectClipBinding(
							Slot.clip, Slot.clipOccurrenceId, Slot.sourceStartMs,
							Slot.playMs, Slot.playRate, Slot.repeatUntilStageEnd,
							iClipWallStartMs, iClipWallEndMs);
						iClipWallCursorMs += iClipWallMs;
					}
				}
				else
				{
					for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
						Stage.ClipOccurrences)
					{
						const uint32_t iClipWallStartMs = static_cast<uint32_t>(
							(std::min)(iClipWallCursorMs,
								static_cast<uint64_t>(iStageDurationMs)));
						const uint32_t iClipWallEndMs = static_cast<uint32_t>(
							(std::min)(iClipWallCursorMs + Clip.iAuthoringWallMs,
								static_cast<uint64_t>(iStageDurationMs)));
						ProjectClipBinding(
							Clip.strClipName, Clip.strClipOccurrenceId,
							Clip.iSourceStartMs, Clip.iPlayMs, Clip.fPlayRate,
							Clip.bLoop, iClipWallStartMs, iClipWallEndMs);
						iClipWallCursorMs += Clip.iAuthoringWallMs;
					}
				}
			}
		}

		if (pPatternSounds && std::any_of(pPatternSounds->Cues.begin(), pPatternSounds->Cues.end(),
			[&](const auto& cue) { return cue.strPatternId == Pattern.strPatternId &&
				cue.strStageId == Stage.strStageId; }))
		{
			std::vector<ACTION_PRESENTATION_CLIP_TIMING> SoundTimings;
			std::vector<std::string> SoundClipIds;
			const auto AppendSoundTiming = [&](const std::string& clipName,
				const std::string& occurrenceId, const uint32_t sourceStartMs,
				const uint32_t playMs, const f32_t playRate, const bool_t loop) {
				const auto [entry, inserted] = SoundClipDurations.try_emplace(clipName, 0.f);
				if (inserted)
				{
					uint32_t roundedMs = 0u;
					std::string status;
					(void)m_pAnimationTool->Resolve_ValtanCompositionNativeClipDurationMs(
						clipName, roundedMs, status, &entry->second);
				}
				SoundTimings.push_back({entry->second, playMs, playRate, loop, sourceStartMs * .001f});
				SoundClipIds.push_back(occurrenceId);
			};
			if (bHasStageDraft)
				for (const auto& clip : StageDraft.animationSlots)
					AppendSoundTiming(clip.clip, clip.clipOccurrenceId, clip.sourceStartMs,
						clip.playMs, static_cast<f32_t>(clip.playRate), clip.repeatUntilStageEnd);
			else
				for (const auto& clip : Stage.ClipOccurrences)
					AppendSoundTiming(clip.strClipName, clip.strClipOccurrenceId, clip.iSourceStartMs,
						clip.iPlayMs, clip.fPlayRate, clip.bLoop);
			for (const VALTAN_PATTERN_SOUND_CUE& Cue : pPatternSounds->Cues)
			{
				if (Cue.strPatternId != Pattern.strPatternId ||
					Cue.strStageId != Stage.strStageId)
					continue;
				uint32_t iLocalMs = ResolveSourceToStageMs(Cue.strClipOccurrenceId, Cue.iStartMs);
				uint32_t iLocalEndMs = iLocalMs;
				const auto clip = std::find(SoundClipIds.begin(), SoundClipIds.end(), Cue.strClipOccurrenceId);
				const auto variants = SoundEvents.find(Cue.strSoundEvent);
				const bool resolved = clip != SoundClipIds.end() && variants != SoundEvents.end() &&
					ResolveValtanSoundTimelineSpan(Cue, SoundTimings,
						static_cast<std::size_t>(clip - SoundClipIds.begin()), iStageDurationMs,
						variants->second, ResolveSoundDuration, iLocalMs, iLocalEndMs);
				const auto endMs = static_cast<uint32_t>((std::min)(
					static_cast<uint64_t>(iStageStartMs) + iLocalEndMs,
					static_cast<uint64_t>((std::numeric_limits<uint32_t>::max)())));
				m_TimelineItems.push_back({
					DETAIL_OWNER::SOUND, TIMELINE_LANE::SOUND, Pattern.strPatternId,
					Stage.strStageId, Cue.strOccurrenceId, Cue.strSoundEvent,
					Cue.strSoundEvent +
						(VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
							Cue.eRepeatPolicy ? " [each_loop]" : " [once]") +
						(resolved ? "" : " [duration unavailable]"),
					iStageStartMs + iLocalMs, endMs, true,
					0u, SOUND_EVENT_MINIMUM_WIDTH_PX });
			}
		}
		for (const auto& cue : Stage.SceneProfileOccurrences)
			m_TimelineItems.push_back({DETAIL_OWNER::SCENE_PROFILE, TIMELINE_LANE::SCENE_PROFILE,
				Pattern.strPatternId, Stage.strStageId, cue.strOccurrenceId, cue.strProfileId,
				cue.strProfileId, iStageStartMs + cue.iStartMs,
				iStageStartMs + cue.iStartMs + cue.iDurationMs, bHasStageDraft});
		for (const auto& cue : Stage.LightOccurrences)
			m_TimelineItems.push_back({DETAIL_OWNER::LIGHT, TIMELINE_LANE::LIGHT,
				Pattern.strPatternId, Stage.strStageId, cue.strOccurrenceId, cue.strLightResourceId,
				cue.strLightResourceId, iStageStartMs + cue.iStartMs,
				iStageStartMs + cue.iStartMs + cue.iDurationMs, bHasStageDraft});
		for (const VALTAN_CAMERA_INVOCATION_VIEW& Camera :
			Stage.CameraInvocations)
		{
			m_TimelineItems.push_back({
				DETAIL_OWNER::CAMERA, TIMELINE_LANE::CAMERA, Pattern.strPatternId,
				Stage.strStageId, Camera.strCameraInvocationId,
				Camera.strCameraCueId, Camera.strCameraCueId,
				iStageStartMs + Camera.iStartOffsetMs,
				iStageStartMs + Camera.iStartOffsetMs + Camera.iDurationMs,
				bHasStageDraft });
		}
		if (m_bPatternShakesReady)
		{
			for (const VALTAN_PATTERN_SHAKE_CUE& Cue : m_PatternShakes.Cues)
			{
				if (Cue.strPatternId != Pattern.strPatternId ||
					Cue.strStageId != Stage.strStageId)
					continue;
				const uint32_t iLocalMs = ResolveSourceToStageMs(
					Cue.strClipOccurrenceId, Cue.iStartMs);
				const int64_t iSignedDurationMs = std::llround(
					static_cast<double>(Cue.Spec.fDurationSeconds) * 1000.0);
				const uint32_t iDurationMs = static_cast<uint32_t>((std::max)(
					int64_t{ 0 }, iSignedDurationMs));
				m_TimelineItems.push_back({
					DETAIL_OWNER::CAMERA, TIMELINE_LANE::CAMERA, Pattern.strPatternId,
					Stage.strStageId, Cue.strOccurrenceId, "camera shake",
					"Camera Shake", iStageStartMs + iLocalMs,
					iStageStartMs + (std::min)(
						iLocalMs + iDurationMs, iStageDurationMs), false });
			}
		}
		for (const VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW& Event :
			Pattern.WorldEventTriggerRefs)
		{
			if (Event.strStageId != Stage.strStageId)
				continue;
			const bool_t bExit = std::string::npos !=
				Event.strTriggerKind.find("EXIT") || std::string::npos !=
				Event.strTriggerKind.find("END");
			const uint32_t iEventMs = bExit ? iStageEndMs : iStageStartMs;
			m_TimelineItems.push_back({
				DETAIL_OWNER::WORLD, TIMELINE_LANE::WORLD, Pattern.strPatternId,
				Stage.strStageId, Event.strTriggerKind, {},
				Event.strTriggerKind, iEventMs, iEventMs, false });
		}
		iStageBaseMs += iStageDurationMs;
	}
	uint64_t iTimelineTailMs = iStageBaseMs;
	for (const TIMELINE_ITEM& Item : m_TimelineItems)
		iTimelineTailMs = (std::max)(
			iTimelineTailMs, static_cast<uint64_t>(Item.iEndMs));
	m_iTimelineDurationMs = static_cast<uint32_t>(
		SaturatingU32(iTimelineTailMs));
	Pack_TimelineSubrows();
	m_strTimelineCachePatternId = Pattern.strPatternId;
	m_eTimelineCachePreviewPath = m_ePreviewPath;
	m_iTimelineCacheDraftGeneration = nullptr == m_pBalanceTool ? 0u :
		m_pBalanceTool->Get_ValtanDraftGeneration();
	m_iTimelineCacheSoundGeneration = nullptr == m_pAnimationTool ? 0u :
		m_pAnimationTool->Get_ValtanCompositionPatternSoundDraftGeneration();
	m_iTimelineCacheEffectV2Revision = CEffectV2Catalog::Get().Get_Revision();
	m_iTimelineCacheEffectV1Revision = CEffectCatalog::Get_RuntimeRevision();
}

void Client::CValtanActionWorkbench::Pack_TimelineSubrows()
{
	m_TimelineLaneSubrowCounts.fill(1u);
	for (std::size_t iLane = 0u; iLane < m_TimelineLaneSubrowCounts.size(); ++iLane)
	{
		const TIMELINE_LANE eLane = static_cast<TIMELINE_LANE>(iLane);
		std::vector<std::size_t> LaneItems;
		for (std::size_t iItem = 0u; iItem < m_TimelineItems.size(); ++iItem)
		{
			if (m_TimelineItems[iItem].eLane == eLane)
				LaneItems.push_back(iItem);
		}
		// Stage는 순서가 있는 한 줄로 배치한다. 확대·축소는 lane 소유권을 바꾸지 않는다.
		if (eLane == TIMELINE_LANE::STAGE)
		{
			for (const std::size_t iItem : LaneItems) m_TimelineItems[iItem].iSubrow = 0u;
			continue;
		}
		std::stable_sort(
			LaneItems.begin(), LaneItems.end(),
			[this](const std::size_t iLeft, const std::size_t iRight)
			{
				const TIMELINE_ITEM& Left = m_TimelineItems[iLeft];
				const TIMELINE_ITEM& Right = m_TimelineItems[iRight];
				if (Left.iStartMs != Right.iStartMs)
					return Left.iStartMs < Right.iStartMs;
				if (Left.iEndMs != Right.iEndMs)
					return Left.iEndMs < Right.iEndMs;
				return Left.strStableId < Right.strStableId;
			});
		std::vector<uint64_t> SubrowVisualEndMs;
		for (const std::size_t iItem : LaneItems)
		{
			TIMELINE_ITEM& Item = m_TimelineItems[iItem];
			const float fDisplayWidthPx = ResolveTimelineDisplayWidthPx(
				Item.strLabel, Item.fMinimumDisplayWidthPx);
			const uint64_t iDisplayDurationMs = static_cast<uint64_t>(
				std::ceil(static_cast<double>(fDisplayWidthPx) * 1000.0 /
					static_cast<double>(m_fTimelinePixelsPerSecond)));
			const uint64_t iVisualEndMs = (std::max)(
				static_cast<uint64_t>(Item.iEndMs),
				static_cast<uint64_t>(Item.iStartMs) + iDisplayDurationMs);
			std::size_t iSubrow = 0u;
			for (; iSubrow < SubrowVisualEndMs.size(); ++iSubrow)
			{
				if (static_cast<uint64_t>(Item.iStartMs) >=
					SubrowVisualEndMs[iSubrow])
				{
					break;
				}
			}
			if (iSubrow == SubrowVisualEndMs.size())
				SubrowVisualEndMs.push_back(iVisualEndMs);
			else
				SubrowVisualEndMs[iSubrow] = iVisualEndMs;
			Item.iSubrow = iSubrow;
		}
		m_TimelineLaneSubrowCounts[iLane] = (std::max)(
			std::size_t{ 1u }, SubrowVisualEndMs.size());
	}
}

bool_t Client::CValtanActionWorkbench::Is_PatternSoundDraftDirty(
	std::string& strOutStatus) const
{
	if (nullptr == m_pAnimationTool)
	{
		strOutStatus = "Pattern Sound owner is unavailable.";
		return false;
	}
	bool_t bDirty = false;
	(void)m_pAnimationTool->Get_ValtanCompositionPatternSoundDraft(
		bDirty, strOutStatus);
	return bDirty;
}

bool_t Client::CValtanActionWorkbench::Has_UnsavedCompositionDrafts(
	std::string& strOutStatus) const
{
	std::vector<std::string> DirtyOwners;
	if (nullptr != m_pBalanceTool && m_pBalanceTool->Is_ValtanDraftDirty())
		DirtyOwners.emplace_back("Pattern / Balance");
	std::string SoundStatus;
	if (Is_PatternSoundDraftDirty(SoundStatus))
		DirtyOwners.emplace_back("Pattern Sound");
	if (m_CombatObjectSounds.bDraftDirty)
		DirtyOwners.emplace_back("Object Sound");
	if (CEffectV2Catalog::Get().Has_BossValtanBindingDraft())
		DirtyOwners.emplace_back("Effect V2");
	if (DirtyOwners.empty())
	{
		strOutStatus.clear();
		return false;
	}
	strOutStatus = "Unsaved owners:";
	for (const std::string& Owner : DirtyOwners)
		strOutStatus += " " + Owner + ";";
	if (!SoundStatus.empty())
		strOutStatus += " " + SoundStatus;
	return true;
}

bool_t Client::CValtanActionWorkbench::
Discard_CompositionDraftsAndReload()
{
	if (nullptr != m_pBalanceTool &&
		(m_pBalanceTool->Is_ValtanSaveJobBlockingAuthoring() ||
		 m_pBalanceTool->Is_ServerRuntimeSetPublishRunning()))
	{
		m_strStatus =
			"Composition discard is blocked until the exact Save/publish receipt is terminal; every draft was preserved.";
		return false;
	}
	if (nullptr == m_pBalanceTool || nullptr == m_pAnimationTool)
	{
		m_strStatus =
			"Pattern switch discard requires the Pattern and Pattern Sound typed owners; the current selection was preserved.";
		return false;
	}
	const bool_t bDiscardObjectSound = m_CombatObjectSounds.bDraftDirty;
	VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT stagedObjectSounds;
	if (bDiscardObjectSound && !CValtanCombatObjectSoundCueDocument::Load_Source(stagedObjectSounds, m_strCombatObjectSoundStatus))
	{ m_strStatus = "Object Sound discard/reload rejected; drafts preserved. " + m_strCombatObjectSoundStatus; return false; }
	const bool_t bDiscardBalance = m_pBalanceTool->Is_ValtanDraftDirty();
	std::string SoundStatus;
	const bool_t bDiscardSound = Is_PatternSoundDraftDirty(SoundStatus);
	const bool_t bDiscardEffectV2 =
		CEffectV2Catalog::Get().Has_BossValtanBindingDraft();
	/* Sound generation 소유권은 변경을 허용할지 먼저 판단하는 공통 조건이다.
	   어느 소유 도구든 draft를 폐기하기 전에 검사해 실행 중인 Server occurrence 때문에 일부만 폐기된 탐색 이동을 줄인다. */
	if (bDiscardSound &&
		!m_pAnimationTool->Can_CommitValtanCompositionPatternSoundGeneration(
			SoundStatus))
	{
		m_strStatus =
			"Pattern switch discard is pinned by the active Pattern Sound generation; the current selection and drafts were preserved. " +
			SoundStatus;
		return false;
	}
	std::string DiscardStatus;
	if (bDiscardBalance &&
		!m_pBalanceTool->Discard_ValtanCompositionDraftAndReload(
			DiscardStatus))
	{
		m_strStatus =
			"Pattern switch could not discard/reload the Balance owner; the current selection was preserved. " +
			DiscardStatus;
		return false;
	}
	if (bDiscardSound &&
		!m_pAnimationTool->
			Discard_ValtanCompositionPatternSoundDraftAndReload(
				DiscardStatus))
	{
		m_strStatus =
			"Pattern switch could not discard/reload the Pattern Sound owner; the current selection was preserved. " +
			DiscardStatus;
		return false;
	}
	if (bDiscardEffectV2 &&
		!CEffectV2Catalog::Get().
			Discard_BossValtanBindingDraftAndReload(DiscardStatus))
	{
		m_strStatus =
			"Pattern switch could not discard/reload the Effect V2 owner; the current selection was preserved. " +
			DiscardStatus;
		return false;
	}
	if (bDiscardObjectSound) m_CombatObjectSounds = std::move(stagedObjectSounds);
	if (!Reload_Canonical())
	{
		m_strStatus =
			"Composition drafts were explicitly discarded, but canonical reload failed; the current stable Pattern selection was preserved. " +
			m_strStatus;
		return false;
	}
	m_strStatus =
		"Discarded every listed Composition owner draft and reloaded the canonical Pattern graph.";
	return true;
}

void Client::CValtanActionWorkbench::Resolve_PendingPatternSelection()
{
	if (!m_PendingPatternSelection.has_value() ||
		PENDING_PATTERN_SELECTION_DECISION::NONE ==
			m_ePendingPatternSelectionDecision)
	{
		return;
	}
	const PENDING_PATTERN_SELECTION Pending = *m_PendingPatternSelection;
	const PENDING_PATTERN_SELECTION_DECISION Decision =
		m_ePendingPatternSelectionDecision;
	m_ePendingPatternSelectionDecision =
		PENDING_PATTERN_SELECTION_DECISION::NONE;
	const std::string strCurrentPatternId = m_strSelectedPatternId;
	const auto PreservePendingFailure =
		[this, &Pending, &strCurrentPatternId](std::string Failure)
	{
		m_bPendingPatternSelectionModalRequested = true;
		m_strStatus =
			"Pattern switch failed; current Pattern " + strCurrentPatternId +
			" and requested Pattern " + Pending.strPatternId +
			" remain selected/pending. " + Failure;
	};

	std::string CompletionPrefix;
	if (PENDING_PATTERN_SELECTION_DECISION::SAVE_AND_SELECT == Decision)
	{
		const bool_t bSaveStarted = Save_Reload();
		m_strPatternSaveStatus = m_strStatus;
		if (!bSaveStarted)
		{
			m_bPatternSaveSucceeded = false;
			m_bPatternSaveResultAvailable = true;
			PreservePendingFailure(m_strPatternSaveStatus);
			return;
		}
		if (0u != m_iPendingSaveJobId)
		{
			/* 선택 이동은 process 실행 성공이 아니라 정확한 작업의 최종 완료 결과에서 이어진다.
			   소유 도구의 수락과 canonical 재로드가 다음 프레임에 끝날 때까지 두 stable ID를 고정한다. */
			m_bSelectAfterPendingSave = true;
			m_bPatternSaveResultAvailable = false;
			return;
		}
		CompletionPrefix =
			"No Composition owner remained dirty; switched to Pattern ";
	}
	else if (PENDING_PATTERN_SELECTION_DECISION::DISCARD_AND_SELECT == Decision)
	{
		if (!Discard_CompositionDraftsAndReload())
		{
			PreservePendingFailure(m_strStatus);
			return;
		}
		CompletionPrefix = "Discarded the listed drafts and switched to Pattern ";
	}
	else
	{
		CompletionPrefix = "Selected Pattern ";
	}

	const VALTAN_PATTERN_VIEW* const pTarget =
		Find_PatternById(Pending.strPatternId);
	const VALTAN_STAGE_VIEW* pTargetStage = nullptr;
	if (nullptr != pTarget && !Pending.strStageId.empty())
	{
		const auto FoundStage = std::find_if(
			pTarget->Stages.begin(), pTarget->Stages.end(),
			[&Pending](const VALTAN_STAGE_VIEW& Stage)
			{ return Stage.strStageId == Pending.strStageId; });
		if (pTarget->Stages.end() != FoundStage)
			pTargetStage = &*FoundStage;
	}
	if (nullptr == pTarget ||
		(!Pending.strStageId.empty() && nullptr == pTargetStage))
	{
		if (PENDING_PATTERN_SELECTION_DECISION::SELECT == Decision)
		{
			m_PendingPatternSelection.reset();
			m_strStatus =
				"Pattern switch rejected a stale requested stable ID; the current selection was preserved: " +
				Pending.strPatternId +
				(Pending.strStageId.empty() ? std::string{} :
					"/" + Pending.strStageId) + ".";
		}
		else
		{
			PreservePendingFailure(
				"The requested stable ID was not present after canonical reload.");
		}
		return;
	}

	if (Pending.bOpenSequencer)
	{
		if (m_pAnimationTool)
		{
			std::string StopStatus;
			m_pAnimationTool->Stop_ValtanCompositionPattern(StopStatus);
		}
		m_bSequencerWindowVisible = true;
	}
	Select_Pattern(*pTarget);
	if (nullptr != pTargetStage)
		Select_Stage(*pTarget, *pTargetStage,
			Pending.eDetailOwner, Pending.strStableId);
	m_PendingPatternSelection.reset();
	m_bPendingPatternSelectionModalRequested = false;
	m_strStatus = CompletionPrefix + Pending.strPatternId +
		(Pending.strStageId.empty() ? std::string{} :
			" / Stage " + Pending.strStageId) + ".";
}

bool_t Client::CValtanActionWorkbench::
Validate_ManualStageTopologySoundDependencies(
	const VALTAN_PATTERN_VIEW& CandidatePattern,
	std::string& strOutStatus) const
{
	if (nullptr == m_pAnimationTool || nullptr == m_pBalanceTool)
	{
		strOutStatus =
			"Manual Stage topology requires both the Pattern owner and Pattern Sound dependency owner.";
		return false;
	}
	std::vector<VALTAN_PATTERN_VIEW> BaselinePatterns;
	std::vector<VALTAN_PATTERN_VIEW> CandidatePatterns;
	const std::vector<const VALTAN_PATTERN_VIEW*> Patterns =
		Collect_CanonicalPatternsForDependencyValidation();
	BaselinePatterns.reserve(Patterns.size());
	CandidatePatterns.reserve(Patterns.size());
	bool_t bCandidateReplaced = false;
	for (const VALTAN_PATTERN_VIEW* const pPattern : Patterns)
	{
		BaselinePatterns.push_back(*pPattern);
		if (pPattern->strPatternId == CandidatePattern.strPatternId)
		{
			CandidatePatterns.push_back(CandidatePattern);
			bCandidateReplaced = true;
			continue;
		}
		VALTAN_PATTERN_VIEW Candidate = *pPattern;
		if (pPattern->bAuthoringMasterManaged &&
			!m_pBalanceTool->Get_ValtanPatternDraft(
				pPattern->strPatternId, Candidate, strOutStatus))
		{
			strOutStatus =
				"Manual Stage topology could not assemble the effective Pattern graph: " +
				strOutStatus;
			return false;
		}
		CandidatePatterns.push_back(std::move(Candidate));
	}
	if (!bCandidateReplaced)
	{
		strOutStatus =
			"Manual Stage topology candidate is not part of the admitted canonical Pattern graph.";
		return false;
	}
	if (!m_pAnimationTool->
			Validate_ValtanCompositionPatternSoundGraphDependencies(
				BaselinePatterns, CandidatePatterns, strOutStatus))
	{
		strOutStatus =
			"Manual Stage topology rejected before the gameplay draft changed: " +
			strOutStatus;
		return false;
	}
	return true;
}

bool_t Client::CValtanActionWorkbench::Save_Reload()
{
	Finish_EditHistory(true);
	const bool_t bPublishAfterSave = std::exchange(m_bPublishWithPendingSave, false);
	Engine::CProfilerScope Profile(
		CGameInstance::Get().Get_Profiler(), "Tool.Composition.SaveReload");
	if (VALTAN_VIEW_ADMISSION::ADMITTED != m_eAdmission ||
		nullptr == m_pBalanceTool)
	{
		m_strStatus =
			"[Pattern] Save is not ready. Reload the Pattern list and try again; no file was changed.";
		return false;
	}
	if (0u != m_iPendingSaveJobId ||
		m_pBalanceTool->Is_ValtanSaveJobBlockingAuthoring() ||
		m_pBalanceTool->Is_ServerRuntimeSetPublishRunning())
	{
		m_strStatus =
			"[Pipeline] Save is blocked while an earlier Save or full DataOnly publish receipt is unresolved; no file was changed.";
		return false;
	}
	std::string CurrentAuthoringRevision;
	std::string CurrentCanonicalSourceRevision;
	std::string CurrentRevisionStatus;
	bool_t bCurrentAuthoringDirty = false;
	if (!m_pBalanceTool->Get_ValtanAuthoringState(
			CurrentAuthoringRevision, bCurrentAuthoringDirty,
			CurrentRevisionStatus) ||
		!m_pBalanceTool->Get_ValtanCanonicalSourceRevision(
			CurrentCanonicalSourceRevision, CurrentRevisionStatus) ||
		CurrentAuthoringRevision != m_strPinnedAuthoringSourceRevision ||
		CurrentCanonicalSourceRevision != m_strPinnedCanonicalSourceRevision)
	{
		m_eAdmission = VALTAN_VIEW_ADMISSION::STALE_PRESERVED;
		m_strStatus =
			"[Pattern] The Pattern files changed after this window opened. Reload, then try again; no file was changed. " +
			CurrentRevisionStatus;
		return false;
	}
    // 모든 pane이 그리기를 마칠 때까지 Save를 대기시킨다. 숨겨진 입력이 있거나
    // 입력 완료 시 검증에 실패했다면 이전 값을 조용히 저장하지 않는다.
    if (!m_strEffectEditIdentity.empty() && m_eDetailOwner == DETAIL_OWNER::EFFECT)
    {
        VALTAN_PATTERN_VIEW DetailPattern;
        if (!m_pBalanceTool->Get_ValtanPatternDraft(m_strSelectedPatternId, DetailPattern, m_strStatus))
            return false;
        if (Has_PendingSelectedEffectDetails(&DetailPattern))
        {
            const auto detailStage = std::find_if(DetailPattern.Stages.begin(), DetailPattern.Stages.end(),
                [&](const auto& row) { return row.strStageId == m_strSelectedStageId; });
            const auto detailCue = std::find_if(detailStage->ProductCues.begin(), detailStage->ProductCues.end(),
                [&](const auto& row) { return row.strOccurrenceId == m_strSelectedStableId; });
            if (!Apply_EffectOccurrenceDetails(DetailPattern, *detailStage, *detailCue, m_EffectCueEditDraft, m_strStatus))
            {
                m_strStatus = "Save canceled: the pending Effect detail could not be applied. " + m_strStatus;
                return false;
            }
            m_strEffectEditIdentity.clear();
            bCurrentAuthoringDirty = m_pBalanceTool->Is_ValtanDraftDirty();
        }
    }
	const bool_t bSavePattern = bCurrentAuthoringDirty;
	std::string SoundStatus;
	const bool_t bSaveSound = Is_PatternSoundDraftDirty(SoundStatus);
	const bool_t bSaveEffectV2 =
		CEffectV2Catalog::Get().Has_BossValtanBindingDraft();
	const bool_t bSaveObjectSound = m_CombatObjectSounds.bDraftDirty;
	const bool_t bSaveShake = m_PatternShakes.bDraftDirty;
	if (!bSavePattern && !bSaveSound && !bSaveEffectV2 && !bSaveShake && !bSaveObjectSound)
	{
		if (bPublishAfterSave)
		{
			std::string PublishableRevision;
			if (!m_pBalanceTool->Get_ValtanPublishSourceRevision(
					PublishableRevision, m_strStatus) ||
				PublishableRevision != CurrentCanonicalSourceRevision)
			{
				m_strStatus = "Publish rejected a changed source revision; local edits were preserved. " + m_strStatus;
				return false;
			}
			m_strPostSaveRevision = PublishableRevision;
			Publish_AfterSave();
			m_strStatus = m_strPostSaveApplyStatus;
			return 0u != m_iPendingSaveJobId;
		}
		m_strStatus = "Source is already saved. Use Save & Publish to update the runtime files.";
		return true;
	}
	if (bSaveSound && nullptr == m_pAnimationTool)
	{
		m_strStatus =
			"[Sound] The changed Sound owner is unavailable; no file was changed.";
		return false;
	}
	// Product와 native resource의 결합은 Publish에서 수행한다.
	// 저장 형식 검증·소유 도구의 기준본·원자적 commit은 source Save transaction이 담당한다.
	std::string PatternSoundBaselineBytes;
	std::string PatternSoundCandidateBytes;
	uint64_t iPatternSoundDraftGeneration = 0u;
	bool_t bPreparedPatternSoundDirty = false;
	if (bSaveSound)
	{
		if (!m_pAnimationTool->Prepare_ValtanCompositionPatternSoundSave(
				PatternSoundBaselineBytes, PatternSoundCandidateBytes,
				iPatternSoundDraftGeneration, bPreparedPatternSoundDirty,
				SoundStatus))
		{
			m_strStatus =
				"[Sound] Sound data could not be staged for the single Save transaction; no file was changed. " +
				SoundStatus;
			return false;
		}
	}
	if (bPreparedPatternSoundDirty != bSaveSound)
	{
		m_strStatus =
			"[Sound] Sound data changed while Save was being prepared; no file was changed. Try Save again.";
		return false;
	}
	std::string EffectV2BaselineBytes;
	std::string EffectV2CandidateBytes;
	std::string EffectV2ReadSetBytes;
	uint64_t iEffectV2DraftRevision = 0u;
	bool_t bEffectV2Dirty = false;
	std::string EffectV2Status;
	if (bSaveEffectV2)
	{
		if (!CEffectV2Catalog::Get().Prepare_BossValtanBindingDraftSave(
				EffectV2BaselineBytes, EffectV2CandidateBytes,
				EffectV2ReadSetBytes,
				iEffectV2DraftRevision, bEffectV2Dirty, EffectV2Status))
		{
			m_strStatus =
				"[EffectV2] Effect V2 data could not be staged for the single Save transaction; no file was changed. " +
				EffectV2Status;
			return false;
		}
	}
	if (bEffectV2Dirty != bSaveEffectV2)
	{
		m_strStatus =
			"[EffectV2] Effect V2 data changed while Save was being prepared; no file was changed. Try Save again.";
		return false;
	}
	std::string PatternShakeBaselineBytes, PatternShakeCandidateBytes;
	uint64_t iPatternShakeDraftGeneration = 0u;
	if (bSaveShake && !CValtanPatternShakeCueDocument::Prepare_Save(m_PatternShakes,
		PatternShakeBaselineBytes, PatternShakeCandidateBytes, iPatternShakeDraftGeneration, m_strShakeStatus))
	{ m_strStatus = "[Camera Shake] Nothing was saved. " + m_strShakeStatus; return false; }
	std::string ObjectSoundBaselineBytes, ObjectSoundCandidateBytes;
	uint64_t iObjectSoundDraftGeneration = 0u;
	if (bSaveObjectSound && !CValtanCombatObjectSoundCueDocument::Prepare_Save(m_CombatObjectSounds,
		ObjectSoundBaselineBytes, ObjectSoundCandidateBytes, iObjectSoundDraftGeneration, m_strCombatObjectSoundStatus))
	{ m_strStatus = "[Object Sound] Nothing was saved. " + m_strCombatObjectSoundStatus; return false; }
	CBalanceTool::VALTAN_COMPOSITION_OWNER_DRAFTS OwnerDrafts;
	if (bSaveObjectSound)
	{
		OwnerDrafts.combatObjectSoundBaselineBytes = std::move(ObjectSoundBaselineBytes);
		OwnerDrafts.combatObjectSoundCandidateBytes = ObjectSoundCandidateBytes;
	}
	if (bSaveShake)
	{
		OwnerDrafts.patternShakeBaselineBytes = std::move(PatternShakeBaselineBytes);
		OwnerDrafts.patternShakeCandidateBytes = PatternShakeCandidateBytes;
	}
	if (bPreparedPatternSoundDirty)
	{
		OwnerDrafts.patternSoundBaselineBytes =
			std::move(PatternSoundBaselineBytes);
		OwnerDrafts.patternSoundCandidateBytes = PatternSoundCandidateBytes;
	}
	if (bEffectV2Dirty)
	{
		OwnerDrafts.effectV2BaselineBytes = std::move(EffectV2BaselineBytes);
		OwnerDrafts.effectV2CandidateBytes = EffectV2CandidateBytes;
		OwnerDrafts.effectV2ReadSetBytes = std::move(EffectV2ReadSetBytes);
	}
	std::uint64_t iSaveJobId = 0u;
	std::string SaveStatus;
	if (!m_pBalanceTool->Begin_ValtanCompositionSave(
			OwnerDrafts, bPublishAfterSave,
			iSaveJobId, SaveStatus))
	{
		m_strStatus = "[Pipeline] Nothing was saved. " +
			SaveStatus;
		return false;
	}
	m_iPendingSaveJobId = iSaveJobId;
	m_iPendingPatternSoundDraftGeneration =
		iPatternSoundDraftGeneration;
	m_iPendingEffectV2DraftRevision = iEffectV2DraftRevision;
	m_iPendingCombatObjectSoundDraftGeneration = iObjectSoundDraftGeneration;
	m_strPendingCombatObjectSoundCandidateBytes = std::move(ObjectSoundCandidateBytes);
	m_bPendingCombatObjectSoundOwner = bSaveObjectSound;
	m_iPendingPatternShakeDraftGeneration = iPatternShakeDraftGeneration;
	m_strPendingPatternShakeCandidateBytes = std::move(PatternShakeCandidateBytes);
	m_bPendingPatternShakeOwner = bSaveShake;
	m_strPendingPatternSoundCandidateBytes =
		std::move(PatternSoundCandidateBytes);
	m_strPendingEffectV2CandidateBytes =
		std::move(EffectV2CandidateBytes);
	m_bPendingPatternSoundOwner = bPreparedPatternSoundDirty;
	m_bPendingEffectV2Owner = bEffectV2Dirty;
	m_bPendingSavePublishesRuntime = bPublishAfterSave;
	m_bPatternSaveResultAvailable = false;
	m_bPatternSaveSucceeded = false;
	m_ePostSaveState = POST_SAVE_STATE::IDLE;
	m_strPostSaveRevision.clear();
	m_strPostSaveApplyStatus.clear();
	m_strStatus = "[Pipeline] " + SaveStatus;
	return true;
}

void Client::CValtanActionWorkbench::Mark_SourceCommitted(
	const std::string& strExactSourceRevision)
{
	m_ePostSaveState = POST_SAVE_STATE::SOURCE_COMMITTED;
	m_strPostSaveRevision = strExactSourceRevision;
}

void Client::CValtanActionWorkbench::Clear_PendingSaveOwnerReceipt()
{
	m_iPendingCombatObjectSoundDraftGeneration = 0u;
	m_strPendingCombatObjectSoundCandidateBytes.clear();
	m_bPendingCombatObjectSoundOwner = false;
	m_iPendingPatternShakeDraftGeneration = 0u;
	m_strPendingPatternShakeCandidateBytes.clear();
	m_bPendingPatternShakeOwner = false;
	m_iPendingPatternSoundDraftGeneration = 0u;
	m_iPendingEffectV2DraftRevision = 0u;
	m_strPendingPatternSoundCandidateBytes.clear();
	m_strPendingEffectV2CandidateBytes.clear();
	m_bPendingPatternSoundOwner = false;
	m_bPendingEffectV2Owner = false;
}

bool_t Client::CValtanActionWorkbench::Accept_PendingSaveOwners(
	std::string& strOutStatus)
{
	bool_t bAccepted = true;
	std::string OwnerStatus;
	if (m_bPendingPatternSoundOwner)
	{
		if (nullptr == m_pAnimationTool ||
			!m_pAnimationTool->Accept_ValtanCompositionPatternSoundSave(
				m_iPendingPatternSoundDraftGeneration,
				m_strPendingPatternSoundCandidateBytes, OwnerStatus))
		{
			bAccepted = false;
			strOutStatus =
				"[Sound] The durable Save completed, but the exact local Pattern Sound generation was not reopened: " +
				(nullptr == m_pAnimationTool ?
					std::string("owner unavailable.") : OwnerStatus);
		}
		else
		{
			m_bPendingPatternSoundOwner = false;
			m_strPendingPatternSoundCandidateBytes.clear();
			m_iPendingPatternSoundDraftGeneration = 0u;
			m_LastPatternSoundAutoApplyRevision = {};
		}
	}
	if (m_bPendingEffectV2Owner)
	{
		OwnerStatus.clear();
		if (!CEffectV2Catalog::Get().Accept_BossValtanBindingDraftSave(
				m_iPendingEffectV2DraftRevision,
				m_strPendingEffectV2CandidateBytes, OwnerStatus))
		{
			bAccepted = false;
			if (!strOutStatus.empty())
				strOutStatus += " ";
			strOutStatus +=
				"[EffectV2] The durable Save completed, but the exact local binding generation was not reopened: " +
				OwnerStatus;
		}
		else
		{
			m_bPendingEffectV2Owner = false;
			m_strPendingEffectV2CandidateBytes.clear();
			m_iPendingEffectV2DraftRevision = 0u;
		}
	}
	if (m_bPendingCombatObjectSoundOwner)
	{
		if (!CValtanCombatObjectSoundCueDocument::Accept_Save(m_CombatObjectSounds,
			m_iPendingCombatObjectSoundDraftGeneration, m_strPendingCombatObjectSoundCandidateBytes, OwnerStatus))
		{ bAccepted = false; strOutStatus += " [Object Sound] " + OwnerStatus; }
		else
		{
			m_bPendingCombatObjectSoundOwner = false; m_iPendingCombatObjectSoundDraftGeneration = 0u;
			m_strPendingCombatObjectSoundCandidateBytes.clear();
		}
	}
	if (m_bPendingPatternShakeOwner)
	{
		if (!CValtanPatternShakeCueDocument::Accept_Save(m_PatternShakes,
			m_iPendingPatternShakeDraftGeneration, m_strPendingPatternShakeCandidateBytes, OwnerStatus))
		{ bAccepted = false; strOutStatus += " [Camera Shake] " + OwnerStatus; }
		else
		{
			m_bPendingPatternShakeOwner = false; m_iPendingPatternShakeDraftGeneration = 0u;
			m_strPendingPatternShakeCandidateBytes.clear();
		}
	}
	return bAccepted;
}

bool_t Client::CValtanActionWorkbench::Reload_AfterPendingSave(
	std::string& strOutStatus)
{
	m_bKeepHistoryOnReload = true;
	const bool_t reopened = Reload_Canonical(!m_bPendingSavePublishesRuntime);
	m_bKeepHistoryOnReload = false;
	strOutStatus = m_strStatus;
	if (reopened && m_bProductSourceReady && m_pValtanBossTool)
	{
		std::string productStatus;
		(void)m_pValtanBossTool->Reload_CanonicalGraph(productStatus);
	}
	return reopened;
}

void Client::CValtanActionWorkbench::Publish_AfterSave()
{
	if (nullptr == m_pBalanceTool)
	{
		m_ePostSaveState = POST_SAVE_STATE::APPLY_FAILED;
		m_strPostSaveApplyStatus =
			"Balance Tool is unavailable; publish the committed revision from the Balance Tool.";
		return;
	}
	if (0u != m_iPendingSaveJobId ||
		m_pBalanceTool->Is_ValtanSaveJobBlockingAuthoring() ||
		m_pBalanceTool->Is_ServerRuntimeSetPublishRunning())
	{
		m_strPostSaveApplyStatus =
			"A Save or runtime publish receipt is still unresolved; Retry Publish once it finishes.";
		return;
	}
	/* 이 후속 처리는 Workbench가 받은 작업 결과의 revision을 기준으로 한다.
	   창이 숨겨진 동안 다른 소유 도구가 더 새 revision을 저장·재로드했을 수 있다.
	   PublishCandidate/Apply의 부작용 전에 그 차이를 거절하며, candidate 단계도 process 경계를 지나므로 이후에 다시 검사한다. */
	std::string PublishableRevision;
	std::string RevisionStatus;
	if (!m_pBalanceTool->Get_ValtanPublishSourceRevision(
			PublishableRevision, RevisionStatus) ||
		PublishableRevision != m_strPostSaveRevision)
	{
		m_ePostSaveState = POST_SAVE_STATE::PUBLISH_FAILED;
		m_strPostSaveApplyStatus =
			"[Apply] STALE_REVISION: the committed Save receipt no longer matches the publishable canonical source. " +
			RevisionStatus;
		return;
	}
	std::uint64_t iSaveJobId = 0u;
	std::string BeginStatus;
	if (!m_pBalanceTool->Begin_ValtanProductPublishRetry(
			true, iSaveJobId, BeginStatus))
	{
		m_ePostSaveState = POST_SAVE_STATE::APPLY_FAILED;
		m_strPostSaveApplyStatus = "[Apply] FAILED: " + BeginStatus;
		return;
	}
	m_iPendingSaveJobId = iSaveJobId;
	m_bPendingSavePublishesRuntime = true;
	m_strPostSaveApplyStatus = "[Apply] " + BeginStatus;
}

void Client::CValtanActionWorkbench::Update_SaveState()
{
	if (nullptr == m_pBalanceTool)
		return;
	if (0u != m_iPendingSaveJobId)
	{
		CBalanceTool::VALTAN_SAVE_JOB_RECEIPT Observed;
		const CBalanceTool::VALTAN_SAVE_JOB_STATE State =
			m_pBalanceTool->Get_ValtanSaveJobState(Observed);
		if (Observed.jobId != m_iPendingSaveJobId)
		{
			m_bPatternSaveSucceeded = false;
			m_bPatternSaveResultAvailable = true;
			m_strPatternSaveStatus =
				"SAVE_RECEIPT_MISMATCH: the active Balance Save job no longer matches this Composition request; local drafts and selection were preserved.";
			m_strStatus = m_strPatternSaveStatus;
			return;
		}
		if (CBalanceTool::VALTAN_SAVE_JOB_STATE::RUNNING == State)
		{
			m_strPatternSaveStatus = Observed.status;
			return;
		}
		if (CBalanceTool::VALTAN_SAVE_JOB_STATE::OBSERVATION_LOST == State)
		{
			m_bPatternSaveSucceeded = false;
			m_bPatternSaveResultAvailable = true;
			m_strPatternSaveStatus = Observed.status;
			m_strStatus = Observed.status;
			return;
		}
		if (CBalanceTool::VALTAN_SAVE_JOB_STATE::IDLE == State)
		{
			m_bPatternSaveSucceeded = false;
			m_bPatternSaveResultAvailable = true;
			m_strPatternSaveStatus =
				"SAVE_RECEIPT_MISSING: Composition retained a pending job ID after the Balance owner returned to IDLE; local drafts were preserved.";
			m_strStatus = m_strPatternSaveStatus;
			return;
		}

		const bool_t bTerminalSucceeded =
			CBalanceTool::VALTAN_SAVE_JOB_STATE::SUCCEEDED == State;
		const bool_t bExactRevision =
			64u == Observed.committedSourceRevision.size();
		std::string PublishableRevision;
		std::string RevisionStatus;
		const bool_t bExactPublishable = bExactRevision &&
			m_pBalanceTool->Get_ValtanPublishSourceRevision(
				PublishableRevision, RevisionStatus) &&
			PublishableRevision == Observed.committedSourceRevision;

		std::string LocalOwnerStatus;
		bool_t bLocalOwnersAccepted = true;
		if ((bTerminalSucceeded || Observed.canonicalCommitted) &&
			bExactRevision)
		{
			bLocalOwnersAccepted =
				Accept_PendingSaveOwners(LocalOwnerStatus);
		}
		CBalanceTool::VALTAN_SAVE_JOB_RECEIPT Receipt;
		std::string ConsumeStatus;
		if (!m_pBalanceTool->Consume_ValtanSaveJobReceipt(
				m_iPendingSaveJobId, Receipt, ConsumeStatus))
		{
			m_bPatternSaveSucceeded = false;
			m_bPatternSaveResultAvailable = true;
			m_strPatternSaveStatus = ConsumeStatus;
			m_strStatus = ConsumeStatus;
			return;
		}
		m_iPendingSaveJobId = 0u;
		bool_t bGraphsReopened = false;
		std::string GraphStatus;
		if (bTerminalSucceeded && bExactPublishable &&
			bLocalOwnersAccepted)
		{
			/* Consume은 Balance 소유 도구를 IDLE로 되돌린다.
			   Save 결과가 아직 활성 상태이면 일반 읽기가 거절되므로 canonical graph 재로드는 이 경계 뒤에서 수행한다. */
			bGraphsReopened = Reload_AfterPendingSave(GraphStatus);
		}

		if ((Receipt.canonicalCommitted ||
			 (!m_strPostSaveRevision.empty() &&
			  m_strPostSaveRevision == Receipt.committedSourceRevision)) &&
			64u == Receipt.committedSourceRevision.size())
		{
			Mark_SourceCommitted(Receipt.committedSourceRevision);
		}

		if (!bTerminalSucceeded || !bExactPublishable)
		{
			m_bPatternSaveSucceeded = false;
			m_bPatternSaveResultAvailable = true;
			m_ePostSaveState = Receipt.canonicalCommitted ?
				POST_SAVE_STATE::APPLY_FAILED : POST_SAVE_STATE::IDLE;
			m_strPostSaveApplyStatus = Receipt.status;
			if (!bExactPublishable && bExactRevision)
			{
				m_strPostSaveApplyStatus +=
					" STALE_REVISION: the terminal Save receipt is not the current durable publish source. " +
					RevisionStatus;
			}
			if (!LocalOwnerStatus.empty())
				m_strPostSaveApplyStatus += " " + LocalOwnerStatus;
			m_strPatternSaveStatus = m_strPostSaveApplyStatus;
			m_strStatus = m_strPatternSaveStatus;
			if (!Receipt.canonicalCommitted)
				Clear_PendingSaveOwnerReceipt();
			if (m_bSelectAfterPendingSave)
			{
				m_bSelectAfterPendingSave = false;
				m_bPendingPatternSelectionModalRequested = true;
			}
			m_bPendingSavePublishesRuntime = false;
			return;
		}

		m_strPostSaveApplyStatus = Receipt.status;
		if (!LocalOwnerStatus.empty())
			m_strPostSaveApplyStatus += " " + LocalOwnerStatus;
		if (!GraphStatus.empty())
			m_strPostSaveApplyStatus += " " + GraphStatus;
		const bool_t bEditorReopened =
			bLocalOwnersAccepted && bGraphsReopened;
		m_bPatternSaveSucceeded = bEditorReopened;
		m_bPatternSaveResultAvailable = true;
		m_strPatternSaveStatus = bEditorReopened ?
			"Source saved; every staged Composition owner reopened the exact committed revision. Publish status is tracked separately." :
			"The canonical Save committed, but one editor-local exact reopen failed; the previous snapshot and any newer draft were preserved. " +
			m_strPostSaveApplyStatus;
		m_strStatus = m_strPatternSaveStatus;

		const bool_t bPublishRuntime =
			m_bPendingSavePublishesRuntime &&
			Receipt.runtimePublishRequested;
		if (bPublishRuntime && !bEditorReopened)
		{
			m_ePostSaveState = POST_SAVE_STATE::PUBLISH_FAILED;
			m_strPostSaveApplyStatus +=
				" [Publish] NOT_STARTED: exact editor graph reopen must succeed before Full DataOnly can consume this revision.";
		}
		else if (bPublishRuntime)
		{
			std::string PublishStatus;
			if (m_pBalanceTool->Publish_ServerRuntimeSet(
					Receipt.committedSourceRevision, PublishStatus))
			{
				m_ePostSaveState = POST_SAVE_STATE::PUBLISH_RUNNING;
				m_strPostSaveApplyStatus += " [Publish] " + PublishStatus;
			}
			else
			{
				m_ePostSaveState = POST_SAVE_STATE::PUBLISH_FAILED;
				m_strPostSaveApplyStatus +=
					" [Publish] FAILED: " + PublishStatus;
			}
		}
		else
		{
			m_ePostSaveState = POST_SAVE_STATE::SOURCE_COMMITTED;
		}

		if (m_bSelectAfterPendingSave)
		{
			m_bSelectAfterPendingSave = false;
			if (bEditorReopened)
			{
				m_ePendingPatternSelectionDecision =
					PENDING_PATTERN_SELECTION_DECISION::SELECT;
			}
			else
			{
				m_bPendingPatternSelectionModalRequested = true;
			}
		}
		Clear_PendingSaveOwnerReceipt();
		m_bPendingSavePublishesRuntime = false;
	}
	Update_PostSaveState();
}

void Client::CValtanActionWorkbench::Update_PostSaveState()
{
	if (POST_SAVE_STATE::PUBLISH_RUNNING != m_ePostSaveState ||
		nullptr == m_pBalanceTool)
	{
		return;
	}
	std::string JobStatus;
	double fElapsedSeconds = 0.0;
	std::string JobRevision;
	switch (m_pBalanceTool->Get_ServerRuntimeSetPublishState(
		JobStatus, fElapsedSeconds, &JobRevision))
	{
	case CBalanceTool::SERVER_RUNTIME_PUBLISH_STATE::SUCCEEDED:
		if (JobRevision != m_strPostSaveRevision)
		{
			m_ePostSaveState = POST_SAVE_STATE::PUBLISH_FAILED;
			m_strPostSaveApplyStatus +=
				" [Publish] STALE_REVISION: the completed runtime job belongs to a different Save receipt.";
		}
		else
		{
			m_ePostSaveState = POST_SAVE_STATE::PUBLISHED_RESTART_REQUIRED;
			m_strPostSaveApplyStatus += " [Publish] " + JobStatus;
			m_strEffectGraphRefreshRevision = m_strPostSaveRevision;
			m_bEffectGraphRefreshRequested = true;
		}
		break;
	case CBalanceTool::SERVER_RUNTIME_PUBLISH_STATE::FAILED:
	case CBalanceTool::SERVER_RUNTIME_PUBLISH_STATE::OBSERVATION_LOST:
		m_ePostSaveState = POST_SAVE_STATE::PUBLISH_FAILED;
		m_strPostSaveApplyStatus += " [Publish] FAILED: " + JobStatus;
		break;
	default:
		break;
	}
}

void Client::CValtanActionWorkbench::Render_PostSaveState()
{
	if (0u != m_iPendingSaveJobId && nullptr != m_pBalanceTool)
	{
		CBalanceTool::VALTAN_SAVE_JOB_RECEIPT Receipt;
		const CBalanceTool::VALTAN_SAVE_JOB_STATE State =
			m_pBalanceTool->Get_ValtanSaveJobState(Receipt);
		const bool_t bObservationLost =
			CBalanceTool::VALTAN_SAVE_JOB_STATE::OBSERVATION_LOST == State;
		ImGui::TextColored(
			bObservationLost ? ImVec4(1.f, 0.45f, 0.35f, 1.f) :
				ImVec4(1.f, 0.72f, 0.25f, 1.f),
			"Save job: %s | id %llu",
			bObservationLost ? "OBSERVATION_LOST" : "RUNNING",
			static_cast<unsigned long long>(m_iPendingSaveJobId));
		if (!Receipt.status.empty())
			ImGui::TextWrapped("%s", Receipt.status.c_str());
	}
	if (POST_SAVE_STATE::IDLE == m_ePostSaveState)
		return;
	const char* pStateName = "";
	ImVec4 Color(0.8f, 0.8f, 0.8f, 1.f);
	switch (m_ePostSaveState)
	{
	case POST_SAVE_STATE::SOURCE_COMMITTED:
		pStateName = "SOURCE_COMMITTED / PUBLISH_PENDING";
		Color = ImVec4(1.f, 0.72f, 0.25f, 1.f);
		break;
	case POST_SAVE_STATE::APPLY_FAILED:
		pStateName = "APPLY_FAILED";
		Color = ImVec4(1.f, 0.45f, 0.35f, 1.f);
		break;
	case POST_SAVE_STATE::PUBLISH_RUNNING:
		pStateName = "PUBLISH_RUNNING";
		Color = ImVec4(1.f, 0.72f, 0.25f, 1.f);
		break;
	case POST_SAVE_STATE::PUBLISH_FAILED:
		pStateName = "PUBLISH_FAILED";
		Color = ImVec4(1.f, 0.45f, 0.35f, 1.f);
		break;
	case POST_SAVE_STATE::PUBLISHED_RESTART_REQUIRED:
		pStateName = "PUBLISHED / SERVER_RESTART_REQUIRED";
		Color = ImVec4(0.35f, 0.86f, 0.45f, 1.f);
		break;
	default:
		break;
	}
	ImGui::TextColored(Color, "Save receipt: %s | revision %s", pStateName,
		m_strPostSaveRevision.empty() ? "(unknown)" : m_strPostSaveRevision.c_str());
	if (POST_SAVE_STATE::PUBLISH_RUNNING == m_ePostSaveState &&
		nullptr != m_pBalanceTool)
	{
		std::string JobStatus;
		double fElapsedSeconds = 0.0;
		(void)m_pBalanceTool->Get_ServerRuntimeSetPublishState(
			JobStatus, fElapsedSeconds);
		ImGui::SameLine();
		ImGui::Text("(%.0f s)", fElapsedSeconds);
	}
	const bool_t bRetryable =
		POST_SAVE_STATE::SOURCE_COMMITTED == m_ePostSaveState ||
		POST_SAVE_STATE::APPLY_FAILED == m_ePostSaveState ||
		POST_SAVE_STATE::PUBLISH_FAILED == m_ePostSaveState;
	if (bRetryable)
	{
		ImGui::SameLine();
		ImGui::BeginDisabled(nullptr == m_pBalanceTool ||
			m_pBalanceTool->Is_ServerRuntimeSetPublishRunning());
		if (ImGui::SmallButton("Retry Publish##CompositionPostSave"))
			Publish_AfterSave();
		ImGui::EndDisabled();
	}
	if (!m_strPostSaveApplyStatus.empty())
		ImGui::TextWrapped("%s", m_strPostSaveApplyStatus.c_str());
	if (POST_SAVE_STATE::PUBLISHED_RESTART_REQUIRED == m_ePostSaveState)
	{
		ImGui::TextDisabled(
			"Restart Server.exe on the server host, re-enter the arena, then Restart Saved Flow on this exact revision.");
	}
}

bool_t Client::CValtanActionWorkbench::Render_Toolbar(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const bool_t bMutationAdmitted)
{
	std::string ServerPlayStatus;
	std::string SoundRuntimeStatus;
	LostArk::Shared::GameplayDataRevision ExpectedServerRevision{};
	/* widget 활성 여부는 메모리 상태만으로 판단한다.
	   실제 Product 종속 파일 묶음의 hash 검사는 명령 실행 시 Boss 소유 도구가 한 번 수행한다. */
	const bool_t bServerRevisionAdmitted = nullptr != m_pValtanBossTool &&
		m_pValtanBossTool->Observe_ServerActivePatternRevision(
			ExpectedServerRevision, ServerPlayStatus);
	bool_t bSoundRuntimeReady = nullptr != m_pAnimationTool &&
		bServerRevisionAdmitted &&
		m_pAnimationTool->Is_ValtanCompositionPatternSoundRuntimeReady(
			ExpectedServerRevision, SoundRuntimeStatus);
	/* Server hot-reload는 Save 프레임 이후에 commit될 수 있다. 새 revision을 관측할 때마다 Sound를 한 번 다시 연결한다.
	   별도 Apply 버튼을 만들거나 매 ImGui 프레임마다 디스크 작업을 반복하지 않는다. */
	if (nullptr != m_pAnimationTool && bServerRevisionAdmitted &&
		!bSoundRuntimeReady &&
		m_LastPatternSoundAutoApplyRevision != ExpectedServerRevision)
	{
		m_LastPatternSoundAutoApplyRevision = ExpectedServerRevision;
		(void)m_pAnimationTool->
			Retry_ValtanCompositionPatternSoundRuntimeApply(
				ExpectedServerRevision, SoundRuntimeStatus);
		bSoundRuntimeReady =
			m_pAnimationTool->Is_ValtanCompositionPatternSoundRuntimeReady(
				ExpectedServerRevision, SoundRuntimeStatus);
	}
	const bool_t bServerPlayAdmitted =
		bServerRevisionAdmitted && bSoundRuntimeReady;
	ImGui::SeparatorText("Server Playback");
	ImGui::TextColored(
		bServerPlayAdmitted ? ImVec4(0.35f, 0.86f, 0.45f, 1.f) :
			ImVec4(1.f, 0.76f, 0.25f, 1.f),
		"Server: %s", bServerPlayAdmitted ? "READY" : "NOT READY");
	if (!bServerPlayAdmitted)
	{
		if (nullptr == pPattern)
		{
			ImGui::TextWrapped("Select a Pattern to enable Server Replay.");
		}
		else if (!bServerRevisionAdmitted &&
			(std::string::npos != ServerPlayStatus.find("presentation generation") ||
			 std::string::npos != ServerPlayStatus.find("physical canonical Product closure")))
		{
			ImGui::TextWrapped(
				"Saved data is newer than this Valtan arena. Re-enter Valtan to use it in Server Replay.");
		}
		else if (!bServerRevisionAdmitted)
		{
			ImGui::TextWrapped(
				"Server Replay is not ready. Start Server and re-enter Valtan.");
		}
		else
		{
			ImGui::TextWrapped(
				"Pattern Sound is not ready. Reopen the Workbench after saving the Sound edit.");
		}
	}
	if (ImGui::BeginTable(
			"##CompositionServerActions", 3,
			ImGuiTableFlags_SizingStretchSame))
	{
		ImGui::TableNextColumn();
		ImGui::BeginDisabled(nullptr == pPattern || !bMutationAdmitted ||
			!bServerPlayAdmitted);
		if (ImGui::Button("Play on Server", ImVec2(-1.f, 0.f)))
		{
			std::string Status;
			(void)Play_ServerVerification(*pPattern, Status);
			m_strStatus = std::move(Status);
		}
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) &&
			!bServerPlayAdmitted)
		{
			ImGui::SetTooltip(
				"Resolve the message above, then try Server Replay again.");
		}
		ImGui::TableNextColumn();
		if (ImGui::Button("Restart", ImVec2(-1.f, 0.f)))
		{
			std::string Status;
			if (nullptr == m_pValtanBossTool ||
				!m_pValtanBossTool->Restart_ServerPattern(
					pPattern->strPatternId, Status))
			{
				m_strStatus = Status;
			}
			else
			{
				m_strStatus = std::move(Status);
			}
		}
		ImGui::TableNextColumn();
		if (ImGui::Button("Queue Next", ImVec2(-1.f, 0.f)))
		{
			std::string Status;
			if (nullptr == m_pValtanBossTool ||
				!m_pValtanBossTool->Queue_NextServerPattern(
					pPattern->strPatternId, Status))
			{
				m_strStatus = std::move(Status);
			}
			else
			{
				m_strStatus = std::move(Status);
			}
		}
		ImGui::EndDisabled();
		ImGui::EndTable();
	}

	ImGui::SeparatorText("Tools");
	if (ImGui::BeginTable(
			"##CompositionToolActions", 3,
			ImGuiTableFlags_SizingStretchSame))
	{
		ImGui::TableNextColumn();
		ImGui::BeginDisabled(!bMutationAdmitted || nullptr == m_pValtanBossTool);
		if (ImGui::Button("Restore Arena", ImVec2(-1.f, 0.f)))
		{
			std::string Status;
			(void)m_pValtanBossTool->Set_ServerArenaPreset(
				LostArk::Shared::VALTAN_ARENA_PRESET::FRESH, Status);
			m_strStatus = std::move(Status);
		}
		ImGui::EndDisabled();
		ImGui::TableNextColumn();
		if (ImGui::Button("Animation Clips", ImVec2(-1.f, 0.f)))
			m_bOpenAnimationToolRequested = true;
		ImGui::TableNextColumn();
		if (ImGui::Button("Pattern Flow", ImVec2(-1.f, 0.f)))
		{
			std::string Status;
#ifdef _DEBUG
			CMainApp* const pApp = CMainApp::Get_Active();
			if (nullptr == pApp || !pApp->Debug_OpenValtanPatternFlow(Status))
				m_strStatus = std::move(Status);
			else
				m_strStatus = std::move(Status);
#else
			Status = "Pattern Flow authoring is unavailable outside a Debug Client.";
			m_strStatus = std::move(Status);
#endif
		}
		ImGui::EndTable();
	}
	return false;
}

void Client::CValtanActionWorkbench::
Render_PendingPatternSelectionModal()
{
	if (m_bPendingPatternSelectionModalRequested &&
		m_PendingPatternSelection.has_value())
	{
		ImGui::OpenPopup(
			"Unsaved Pattern Changes###CompositionPatternSelectionUnsaved");
		m_bPendingPatternSelectionModalRequested = false;
	}
	if (!ImGui::BeginPopupModal(
			"Unsaved Pattern Changes###CompositionPatternSelectionUnsaved",
			nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		return;
	}
	if (!m_PendingPatternSelection.has_value())
	{
		ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
		return;
	}

	const PENDING_PATTERN_SELECTION& Pending =
		*m_PendingPatternSelection;
	ImGui::TextUnformatted(
		"The current Composition has unsaved owner drafts.");
	ImGui::Separator();
	ImGui::Text("Current: %s", m_strSelectedPatternId.empty() ?
		"(none)" : m_strSelectedPatternId.c_str());
	ImGui::Text("Load:    %s", Pending.strPatternId.c_str());
	if (!Pending.strStageId.empty())
		ImGui::Text("Stage:   %s", Pending.strStageId.c_str());
	ImGui::SeparatorText("Drafts that will be resolved");
	bool_t bListedDraft = false;
	if (nullptr != m_pBalanceTool && m_pBalanceTool->Is_ValtanDraftDirty())
	{
		ImGui::BulletText("Pattern / Balance owner");
		bListedDraft = true;
	}
	std::string SoundStatus;
	if (Is_PatternSoundDraftDirty(SoundStatus))
	{
		ImGui::BulletText("Pattern Sound owner");
		bListedDraft = true;
	}
	if (m_CombatObjectSounds.bDraftDirty)
	{
		ImGui::BulletText("Object Sound owner");
		bListedDraft = true;
	}
	if (CEffectV2Catalog::Get().Has_BossValtanBindingDraft())
	{
		ImGui::BulletText("Effect V2 BOSS_VALTAN binding owner");
		bListedDraft = true;
	}
	if (!bListedDraft)
	{
		ImGui::TextDisabled(
			"No owner currently reports dirty; retry the requested switch or Cancel.");
	}

	const bool_t bTransactionBlocked = nullptr != m_pBalanceTool &&
		(m_pBalanceTool->Is_ValtanSaveJobBlockingAuthoring() ||
		 m_pBalanceTool->Is_ServerRuntimeSetPublishRunning());
	ImGui::BeginDisabled(bTransactionBlocked);
	if (ImGui::Button("Save All & Switch"))
	{
		m_ePendingPatternSelectionDecision =
			PENDING_PATTERN_SELECTION_DECISION::SAVE_AND_SELECT;
		ImGui::CloseCurrentPopup();
	}
	ImGui::SameLine();
	if (ImGui::Button("Discard Listed Drafts & Switch"))
	{
		m_ePendingPatternSelectionDecision =
			PENDING_PATTERN_SELECTION_DECISION::DISCARD_AND_SELECT;
		ImGui::CloseCurrentPopup();
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Cancel"))
	{
		const std::string strCancelledTarget = Pending.strPatternId;
		m_PendingPatternSelection.reset();
		m_ePendingPatternSelectionDecision =
			PENDING_PATTERN_SELECTION_DECISION::NONE;
		m_bPendingPatternSelectionModalRequested = false;
		m_strStatus =
			"Cancelled Pattern switch to " + strCancelledTarget +
			"; the current selection and every listed draft were preserved.";
		ImGui::CloseCurrentPopup();
	}
	ImGui::EndPopup();
}

void Client::CValtanActionWorkbench::Render_Browser(
	const VALTAN_PATTERN_VIEW* const pEffectiveSelectedPattern)
{
	ImGui::SeparatorText("Pattern / Stage Browser");
	ImGui::BeginDisabled(nullptr == pEffectiveSelectedPattern);
	if (ImGui::Button("Open Boss Pattern"))
	{
		m_bBossPatternWindowVisible = true;
		m_bBossPatternFocusRequested = true;
		m_bBossPatternFitRequested = true;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled("Logic branches + Stage structure");
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint(
		"##CompositionPatternSearch", "Search Pattern ID or display name...",
		m_PatternSearch.data(), m_PatternSearch.size());
	if (m_bProductFallbackReady && 0u == m_CanonicalView.Get_PatternCount())
	{
		Render_ProductFallbackBrowser();
		return;
	}
	const std::string Query = m_PatternSearch.data();
	const std::vector<const VALTAN_PATTERN_VIEW*> Patterns =
		Collect_CanonicalPatternsForDependencyValidation();
	const std::size_t iEditablePatternCount = static_cast<std::size_t>(std::count_if(
		Patterns.begin(), Patterns.end(), [](const VALTAN_PATTERN_VIEW* const Pattern)
		{ return nullptr != Pattern && Pattern->bAuthoringMasterManaged; }));
	ImGui::TextDisabled("%zu saved patterns | %zu editable | %zu Server playable | %zu compatibility references",
		Patterns.size(), iEditablePatternCount, m_PlayableInventory.Get_PatternCount(),
		Patterns.size() - iEditablePatternCount);
	for (const VALTAN_PATTERN_VIEW* const pPattern : Patterns)
	{
		if (nullptr == pPattern ||
			(!ContainsInsensitive(pPattern->strPatternId, Query) &&
			 !ContainsInsensitive(pPattern->strDisplayName, Query)))
			continue;
		ImGui::PushID(pPattern->strPatternId.c_str());
		const bool_t bSelected =
			pPattern->strPatternId == m_strSelectedPatternId;
		const VALTAN_PATTERN_VIEW* const pDisplayPattern =
			bSelected && nullptr != pEffectiveSelectedPattern &&
			pEffectiveSelectedPattern->strPatternId == pPattern->strPatternId ?
			pEffectiveSelectedPattern : pPattern;
		const std::string Label = pDisplayPattern->strDisplayName +
			(!pDisplayPattern->bAuthoringMasterManaged ? " [compatibility reference]##pattern" :
				m_PlayableInventory.Contains(pDisplayPattern->strPatternId) ?
					"##pattern" : " [authoring draft]##pattern");
		const ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_OpenOnArrow |
			ImGuiTreeNodeFlags_SpanAvailWidth |
			(bSelected ? ImGuiTreeNodeFlags_Selected : 0);
		const bool_t bOpen = ImGui::TreeNodeEx(Label.c_str(), Flags);
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
			Request_PatternSelection(*pDisplayPattern);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("%s", pDisplayPattern->strPatternId.c_str());
		if (bOpen)
		{
			if (!pDisplayPattern->bAuthoringMasterManaged)
				ImGui::TextWrapped("Saved compatibility definition: inspect tracks and Preview here. "
					"Editing and Server Play belong to the current split-owned Patterns.");
			ImGui::TextDisabled(
				"%s | phase %u..%u | %zu stages",
				pDisplayPattern->strPatternId.c_str(),
				pDisplayPattern->iMinimumPhase,
				pDisplayPattern->iMaximumPhase,
				pDisplayPattern->Stages.size());
			for (const VALTAN_STAGE_VIEW& Stage : pDisplayPattern->Stages)
			{
				ImGui::PushID(Stage.strStageId.c_str());
				const bool_t bStageSelected = bSelected &&
					Stage.strStageId == m_strSelectedStageId;
				const std::string StageLabel = Stage.strStageId + " | " +
					StageSequenceRoleLabel(Stage) + " | " +
					std::to_string(Stage.iDurationMs) + " ms";
				if (ImGui::Selectable(
						StageLabel.c_str(), bStageSelected,
						0))
				{
					if (bSelected)
						Select_Stage(*pDisplayPattern, Stage);
					else
						Request_PatternSelection(
							*pDisplayPattern, Stage.strStageId);
				}
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
}

void Client::CValtanActionWorkbench::Render_WorldObjectResources()
{
	ImGui::TextWrapped("Select a saved World Object to open its owning Pattern and Stage. "
		"Play on Server replays its spawn, Effect and hit clocks. Local Play previews the supported Animation and Effect tracks.");
	ImGui::SetNextItemWidth(-1.f);
	ImGui::InputTextWithHint("##ValtanWorldObjectSearch", "Search object, Pattern or World event...",
		m_WorldObjectSearch.data(), m_WorldObjectSearch.size());
	const std::string Query = m_WorldObjectSearch.data();
	for (const VALTAN_PATTERN_VIEW* const SavedPattern : Collect_Patterns())
	{
		const VALTAN_PATTERN_VIEW& Pattern =
			SavedPattern->strPatternId == m_strSelectedPatternId && m_pWorkbenchFramePattern ?
			*m_pWorkbenchFramePattern : *SavedPattern;
		ImGui::PushID(Pattern.strPatternId.c_str());
		for (const VALTAN_STAGE_VIEW& Stage : Pattern.Stages)
		{
			ImGui::PushID(Stage.strStageId.c_str());
			for (const VALTAN_COMBAT_OBJECT_EFFECT_VIEW& Object : Stage.CombatObjectEffects)
			{
				if (!ContainsInsensitive(Object.strCombatObjectArchetypeId, Query) &&
					!ContainsInsensitive(Pattern.strPatternId, Query) &&
					!ContainsInsensitive(Pattern.strDisplayName, Query))
					continue;
				ImGui::PushID(Object.strCombatObjectArchetypeId.c_str());
				const std::string Label = Object.strCombatObjectArchetypeId + " | " +
					Pattern.strDisplayName + " / " + Stage.strStageId;
				const bool Selected = Pattern.strPatternId == m_strSelectedPatternId &&
					Stage.strStageId == m_strSelectedStageId &&
					DETAIL_OWNER::COMBAT_OBJECT == m_eDetailOwner &&
					Object.strCombatObjectArchetypeId == m_strSelectedStableId;
				if (ImGui::Selectable(Label.c_str(), Selected))
				{
					if (Pattern.strPatternId == m_strSelectedPatternId)
						Select_Stage(Pattern, Stage, DETAIL_OWNER::COMBAT_OBJECT,
							Object.strCombatObjectArchetypeId);
					else
						Request_PatternSelection(Pattern, Stage.strStageId,
							DETAIL_OWNER::COMBAT_OBJECT, Object.strCombatObjectArchetypeId);
					m_bDetailsWindowVisible = true;
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("%s | origin %s | life %u ms | spawn +%u ms",
						Object.strEffectV2GroupId.empty() ? Object.strEffectAssetId.c_str() :
						Object.strEffectV2GroupId.c_str(), Object.strOriginPolicy.c_str(),
						Object.iLifetimeMs, Object.iFirstSpawnOffsetMs);
				ImGui::PopID();
			}
			for (const VALTAN_WORLD_EVENT_TRIGGER_REF_VIEW& Event : Pattern.WorldEventTriggerRefs)
			{
				if (Event.strStageId != Stage.strStageId ||
					(!ContainsInsensitive(Event.strTriggerKind, Query) &&
					 !ContainsInsensitive(Pattern.strPatternId, Query)))
					continue;
				const std::string Label = "World event " + Event.strTriggerKind + " | " +
					Pattern.strDisplayName + " / " + Stage.strStageId;
				if (ImGui::Selectable(Label.c_str(), false))
				{
					if (Pattern.strPatternId == m_strSelectedPatternId)
						Select_Stage(Pattern, Stage, DETAIL_OWNER::WORLD, Event.strTriggerKind);
					else
						Request_PatternSelection(Pattern, Stage.strStageId,
							DETAIL_OWNER::WORLD, Event.strTriggerKind);
					m_bDetailsWindowVisible = true;
				}
			}
			ImGui::PopID();
		}
		ImGui::PopID();
	}
}

void Client::CValtanActionWorkbench::Render_ProductFallbackBrowser()
{
	const std::string Query = m_PatternSearch.data();
	const auto& Patterns = m_ProductFallbackEncounter.Get_Patterns();
	ImGui::TextColored(
		ImVec4(1.f, 0.72f, 0.18f, 1.f),
		"READ-ONLY PRODUCT FALLBACK");
	ImGui::TextDisabled("%zu generated Product patterns", Patterns.size());
	ImGui::TextWrapped(
		"Pattern and Stage identities remain visible, but Logic branch authoring, preview, Save, and Server playback require the strict canonical join.");
	for (const ENCOUNTER_PATTERN_REFERENCE& Pattern : Patterns)
	{
		if (!Query.empty() &&
			!ContainsInsensitive(Pattern.patternId, Query) &&
			!ContainsInsensitive(Pattern.displayName, Query) &&
			!ContainsInsensitive(Pattern.actionId, Query))
		{
			continue;
		}
		ImGui::PushID(Pattern.patternId.c_str());
		const bool_t bSelected =
			Pattern.patternId == m_strSelectedPatternId;
		const std::string Label =
			(Pattern.displayName.empty() ? Pattern.patternId :
				Pattern.displayName) + "##productFallbackPattern";
		const bool_t bOpen = ImGui::TreeNodeEx(
			Label.c_str(), ImGuiTreeNodeFlags_OpenOnArrow |
				ImGuiTreeNodeFlags_SpanAvailWidth |
				(bSelected ? ImGuiTreeNodeFlags_Selected : 0));
		if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
		{
			m_strSelectedPatternId = Pattern.patternId;
			m_strSelectedStageId = Pattern.stages.empty() ?
				std::string{} : Pattern.stages.front().stageId;
			m_strSelectedStableId = Pattern.patternId;
			m_eDetailOwner = DETAIL_OWNER::PATTERN;
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Product Pattern ID: %s",
				Pattern.patternId.c_str());
		if (bOpen)
		{
			ImGui::TextDisabled("%s | %zu stages | %u ms",
				Pattern.patternId.c_str(), Pattern.stages.size(),
				Pattern.iTotalDurationMs);
			for (const ENCOUNTER_STAGE_REFERENCE& Stage : Pattern.stages)
			{
				const bool_t bStageSelected = bSelected &&
					Stage.stageId == m_strSelectedStageId;
				const std::string StageLabel = Stage.stageId + " | " +
					Stage.stageKind + " | " +
					std::to_string(Stage.iDurationMs) + " ms" +
					(Stage.bHasCounterHitBranch ? " | COUNTER" : "");
				if (ImGui::Selectable(StageLabel.c_str(), bStageSelected))
				{
					m_strSelectedPatternId = Pattern.patternId;
					m_strSelectedStageId = Stage.stageId;
					m_strSelectedStableId = Stage.stageId;
					m_eDetailOwner = DETAIL_OWNER::GAMEPLAY_STAGE;
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Product action: %s",
						Stage.actionId.c_str());
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
}

void Client::CValtanActionWorkbench::Render_SequenceBrowser(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const VALTAN_STAGE_VIEW* const pStage,
	const bool_t bMutationAdmitted)
{
	if (!m_bAnimationSequenceLoadAttempted)
		(void)Reload_AnimationSequences();
	if (ImGui::Button("Reload Animation Sequences"))
		(void)Reload_AnimationSequences();
	ImGui::TextWrapped("%s", m_strAnimationSequenceStatus.c_str());
	ImGui::SetNextItemWidth(-1.f);
	if (ImGui::InputTextWithHint(
		"##CompositionSequenceSearch", "Search action, mode or clip...",
		m_ResourceSearch.data(), m_ResourceSearch.size()))
	{
		m_bAnimationSequenceFilterDirty = true;
	}
	const std::string Query = m_ResourceSearch.data();
	if (m_bAnimationSequenceFilterDirty ||
		m_strAnimationSequenceFilterQuery != Query)
	{
		m_FilteredAnimationSequenceIndices.clear();
		m_FilteredAnimationSequenceIndices.reserve(m_AnimationSequences.size());
		for (std::size_t iSequence = 0u;
			iSequence < m_AnimationSequences.size(); ++iSequence)
		{
			const CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW& Sequence =
				m_AnimationSequences[iSequence];
			const bool_t bMatches =
				ContainsInsensitive(Sequence.strDisplayName, Query) ||
				ContainsInsensitive(Sequence.strMode, Query) ||
				ContainsInsensitive(Sequence.strProfileId, Query) ||
				std::any_of(
					Sequence.Clips.begin(), Sequence.Clips.end(),
					[&Query](
						const CAnimation_Tool::COMPOSITION_SEQUENCE_CLIP_VIEW& Clip)
					{
						return ContainsInsensitive(Clip.strClipName, Query);
					});
			if (bMatches)
				m_FilteredAnimationSequenceIndices.push_back(iSequence);
		}
		m_AnimationResourceTree = {};
		for (const std::size_t iSequence :
			m_FilteredAnimationSequenceIndices)
		{
			const CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW& Sequence =
				m_AnimationSequences[iSequence];
			InsertResourceTree(
				m_AnimationResourceTree,
				{ Sequence.strCategory.empty() ? "UNCATEGORIZED" :
						Sequence.strCategory,
				  Sequence.strProfileId.empty() ? "UNKNOWN PROFILE" :
					Sequence.strProfileId,
				  Sequence.strMode.empty() ? "UNCLASSIFIED" : Sequence.strMode },
				iSequence);
		}
		(void)FinalizeResourceTree(m_AnimationResourceTree);
		m_strAnimationSequenceFilterQuery = Query;
		m_bAnimationSequenceFilterDirty = false;
		const auto VisibleSelection = std::find_if(
			m_FilteredAnimationSequenceIndices.begin(),
			m_FilteredAnimationSequenceIndices.end(),
			[this](const std::size_t iSequence)
			{
				return m_AnimationSequences[iSequence].strStableId ==
					m_strSelectedSequenceStableId;
			});
		if (VisibleSelection == m_FilteredAnimationSequenceIndices.end() &&
			!m_FilteredAnimationSequenceIndices.empty())
		{
			const CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW& First =
				m_AnimationSequences[m_FilteredAnimationSequenceIndices.front()];
			m_strSelectedSequenceStableId = First.strStableId;
			m_iSelectedSequenceSkillId = First.iSkillId;
			m_iSelectedSequenceIndex = First.iSequenceIndex;
		}
	}
	ImGui::TextDisabled(
		"%zu matching source Sequences", m_FilteredAnimationSequenceIndices.size());

	const auto Selected = std::find_if(
		m_AnimationSequences.begin(), m_AnimationSequences.end(),
		[this](const CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW& Sequence)
		{
			return Sequence.strStableId == m_strSelectedSequenceStableId;
		});
	const CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW* pSelected =
		m_AnimationSequences.end() == Selected ? nullptr : &*Selected;
	if (nullptr != pSelected)
	{
		ImGui::SeparatorText("Selected Sequence");
		ImGui::TextWrapped("%s", pSelected->strDisplayName.c_str());
		ImGui::TextDisabled("Action %d | Sequence %d | %s",
			pSelected->iSkillId, pSelected->iSequenceIndex,
			pSelected->strMode.c_str());
		ImGui::TextDisabled("Category: %s | Profile: %s",
			pSelected->strCategory.c_str(), pSelected->strProfileId.c_str());
		for (std::size_t iClip = 0u; iClip < pSelected->Clips.size(); ++iClip)
		{
			const CAnimation_Tool::COMPOSITION_SEQUENCE_CLIP_VIEW& Clip =
				pSelected->Clips[iClip];
			ImGui::BulletText("%02zu  %s  | %u ms%s", iClip + 1u,
				Clip.strClipName.c_str(), Clip.iDurationMs,
				Clip.bUsesNativeDuration ? " native" : " cut");
		}
		const CAnimation_Tool::COMPOSITION_PREVIEW_STATE SourcePreview =
			nullptr == m_pAnimationTool ?
				CAnimation_Tool::COMPOSITION_PREVIEW_STATE{} :
				m_pAnimationTool->Get_ValtanCompositionPreviewState();
		ImGui::TextColored(
			SourcePreview.bModelReady ?
				ImVec4(0.35f, 0.86f, 0.45f, 1.f) :
				ImVec4(1.f, 0.72f, 0.24f, 1.f),
			"Preview target: ARENA CLONE %s | Source: %s | Server Valtan: UNCHANGED",
			SourcePreview.bModelReady ? "READY" : "WILL STAGE ON CLICK",
			SourcePreview.bSourceSequencePlaying ? "PLAYING" : "IDLE");
		if (!SourcePreview.strSourceSequenceStatus.empty())
			ImGui::TextWrapped("%s",
				SourcePreview.strSourceSequenceStatus.c_str());
		ImGui::TextDisabled(
			"Raw Sequence preview needs only its admitted source catalog and Valtan CModel; it does not require a canonical Pattern revision.");
		ImGui::BeginDisabled(nullptr == m_pAnimationTool);
		if (ImGui::Button("Preview Sequence on Arena Clone"))
		{
			std::string Status;
			/* Arena·모델·source 조회 실패만으로 다른 도구의 viewport 입력을 가져오지 않는다.
			   local clone transaction이 실제로 시작된 뒤에만 Composition의 입력 소유권을 요청한다. */
			if (m_pAnimationTool->Preview_ValtanCompositionSequence(
					pSelected->iSkillId, pSelected->iSequenceIndex, Status))
			{
				m_bPreviewOwnerClaimRequested = true;
			}
			m_strStatus = std::move(Status);
		}
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			ImGui::SetTooltip(
				"Spawns/uses the collision-free local Valtan Model View in the current Valtan Arena. This does not mutate the Server boss.");
		}
		ImGui::BeginDisabled(nullptr == pPattern || nullptr == pStage ||
			!bMutationAdmitted || nullptr == m_pBalanceTool);
		if (ImGui::Button("Replace Stage Slots"))
			(void)Apply_SelectedSequenceToStage(*pPattern, *pStage, false);
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(nullptr == pPattern || !bMutationAdmitted || nullptr == m_pBalanceTool);
		if (ImGui::Button("Append to Pattern End"))
		{
			(void)Append_SelectedSequenceToPattern(*pPattern);
			ImGui::EndDisabled();
			return;
		}
		ImGui::EndDisabled();
		ImGui::BeginDisabled(nullptr == m_pAnimationTool || !bMutationAdmitted);
		if (ImGui::Button("Use for Create New Pattern"))
		{
			std::string Status;
			if (m_pAnimationTool->Stage_ValtanCompositionIntakeSequence(
				pSelected->iSkillId, pSelected->iSequenceIndex,
				nullptr == pPattern ? std::string{} : pPattern->strPatternId,
				nullptr == pStage ? std::string{} : pStage->strStageId,
				Status))
			{
				m_bPatternsWindowVisible = true;
				m_iRequestedPatternTab = 1;
			}
			m_strStatus = std::move(Status);
		}
		ImGui::EndDisabled();

		const SOURCE_SEQUENCE_OWNER_INDEX_ENTRY* pSourceOwnerEntry = nullptr;
		if (pSelected->iSkillId >= 0 && pSelected->iSequenceIndex >= 0)
		{
			Ensure_SourceSequenceOwnerIndex();
			pSourceOwnerEntry = Find_SourceSequenceOwners(
				static_cast<uint32_t>(pSelected->iSkillId),
				static_cast<uint32_t>(pSelected->iSequenceIndex));
		}
		static const std::vector<const VALTAN_PATTERN_VIEW*> NoSourceOwners;
		const std::vector<const VALTAN_PATTERN_VIEW*>& SourceOwners =
			nullptr == pSourceOwnerEntry ? NoSourceOwners :
				pSourceOwnerEntry->Owners;
		const auto SelectedSourceOwner = std::find_if(
			SourceOwners.begin(), SourceOwners.end(),
			[this](const VALTAN_PATTERN_VIEW* const pCandidate)
			{
				return nullptr != pCandidate &&
					pCandidate->strPatternId ==
						m_strSourceSequenceServerPatternId;
			});
		if (!SourceOwners.empty() && SelectedSourceOwner == SourceOwners.end())
		{
			const auto CurrentPatternOwner = std::find_if(
				SourceOwners.begin(), SourceOwners.end(),
				[pPattern](const VALTAN_PATTERN_VIEW* const pCandidate)
				{
					return nullptr != pPattern && nullptr != pCandidate &&
						pCandidate->strPatternId == pPattern->strPatternId;
				});
			m_strSourceSequenceServerPatternId =
				(CurrentPatternOwner == SourceOwners.end() ?
					SourceOwners.front() : *CurrentPatternOwner)->strPatternId;
		}
		const VALTAN_PATTERN_VIEW* pServerPattern = nullptr;
		for (const VALTAN_PATTERN_VIEW* const pCandidate : SourceOwners)
		{
			if (nullptr != pCandidate && pCandidate->strPatternId ==
				m_strSourceSequenceServerPatternId)
			{
				pServerPattern = pCandidate;
				break;
			}
		}
		if (SourceOwners.size() > 1u)
		{
			const char_t* const pPreview = nullptr == pServerPattern ?
				"Select owning Pattern" : pServerPattern->strDisplayName.c_str();
			ImGui::SetNextItemWidth(-1.f);
			if (ImGui::BeginCombo("Server Pattern##SourceSequenceOwner", pPreview))
			{
				for (const VALTAN_PATTERN_VIEW* const pCandidate : SourceOwners)
				{
					if (nullptr == pCandidate)
						continue;
					const bool_t bSelectedOwner = pCandidate == pServerPattern;
					const std::string Label = pCandidate->strDisplayName + " | " +
						pCandidate->strPatternId;
					if (ImGui::Selectable(Label.c_str(), bSelectedOwner))
						m_strSourceSequenceServerPatternId =
							pCandidate->strPatternId;
					if (bSelectedOwner)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}
		else if (nullptr != pServerPattern)
		{
			ImGui::TextDisabled("Owning saved Pattern: %s | %s",
				pServerPattern->strDisplayName.c_str(),
				pServerPattern->strPatternId.c_str());
		}
		ImGui::BeginDisabled(nullptr == pServerPattern || VALTAN_VIEW_ADMISSION::ADMITTED != m_eAdmission);
		if (ImGui::Button("Open Owning Pattern in Sequencer"))
			Request_PatternSelection(*pServerPattern, {}, DETAIL_OWNER::PATTERN, {}, true);
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Open this animation's complete Pattern, including its saved V1/V2 effects, timing and preview. Unsaved edits are preserved by the Pattern switch dialog.");
		ImGui::EndDisabled();
		std::string ServerPlayStatus;
		std::string SoundRuntimeStatus;
		LostArk::Shared::GameplayDataRevision ExpectedServerRevision{};
		const bool_t bServerRevisionAdmitted = nullptr != m_pValtanBossTool &&
			m_pValtanBossTool->Observe_ServerActivePatternRevision(
				ExpectedServerRevision, ServerPlayStatus);
		const bool_t bSoundRuntimeReady = nullptr != m_pAnimationTool &&
			bServerRevisionAdmitted &&
			m_pAnimationTool->Is_ValtanCompositionPatternSoundRuntimeReady(
				ExpectedServerRevision, SoundRuntimeStatus);
		const bool_t bCanPlayServerPattern = nullptr != pServerPattern &&
			bServerRevisionAdmitted && bSoundRuntimeReady;
		ImGui::BeginDisabled(!bCanPlayServerPattern);
		if (ImGui::Button(
				"Play Owning Saved Active Revision on Server Valtan"))
		{
			std::string Status;
#ifdef _DEBUG
			if (CMainApp* const pApp = CMainApp::Get_Active();
				nullptr != pApp &&
				pApp->Debug_SelectCompletePlayPattern(
					pServerPattern->strPatternId))
			{
				(void)pApp->Debug_CompletePlaySelected(Status);
			}
			else
			{
				Status =
					"Server Pattern Play selection was rejected by the admitted inventory.";
			}
#else
			Status =
				"Server Pattern Play is unavailable outside a Debug Client.";
#endif
			m_strStatus = std::move(Status);
		}
		ImGui::EndDisabled();
		if (!SourceOwners.empty())
		{
			ImGui::TextDisabled(
				"Server target: SAVED ACTIVE REVISION. Unsaved local Pattern drafts are not sent.");
		}
		if (SourceOwners.empty())
		{
			ImGui::TextDisabled(
				"No saved Pattern owns this raw source. Use Create New Pattern, Save, then Server Play.");
		}
		else if (!bServerRevisionAdmitted && !ServerPlayStatus.empty())
		{
			ImGui::TextDisabled("Server Play unavailable: %s",
				ServerPlayStatus.c_str());
		}
		else if (!bSoundRuntimeReady && !SoundRuntimeStatus.empty())
		{
			ImGui::TextDisabled("Server Play unavailable: %s",
				SoundRuntimeStatus.c_str());
		}
	}

	ImGui::SeparatorText("Animation Sequence Tree");
	/* 선택 Sequence의 동작 버튼을 이 높이가 제한된 스크롤 영역 위에 둔다.
	   420617/1처럼 아래쪽 항목을 선택해도 Preview·Replace·Append 버튼이 Resources 창 밖으로 밀리지 않게 한다. */
	ImGui::BeginChild(
		"##AnimationSequenceTree", ImVec2(0.f, 0.f), true);
	RenderResourceTree(
		m_AnimationResourceTree,
		[this, &pSelected](const std::size_t iSequence)
		{
			const CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW& Sequence =
				m_AnimationSequences[iSequence];
			ImGui::PushID(Sequence.strStableId.c_str());
			const bool_t bSelected = Sequence.strStableId ==
				m_strSelectedSequenceStableId;
			const std::string Label = Sequence.strDisplayName + " | " +
				std::to_string(Sequence.Clips.size()) + " clips";
			if (ImGui::Selectable(Label.c_str(), bSelected))
			{
				m_strSelectedSequenceStableId = Sequence.strStableId;
				m_iSelectedSequenceSkillId = Sequence.iSkillId;
				m_iSelectedSequenceIndex = Sequence.iSequenceIndex;
				pSelected = &Sequence;
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s | source action %d / sequence %d",
					Sequence.strProfileId.c_str(), Sequence.iSkillId,
					Sequence.iSequenceIndex);
			ImGui::PopID();
		});
	ImGui::EndChild();
}

bool_t Client::CValtanActionWorkbench::Validate_TimelineDependencyWindows(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const VALTAN_PRODUCT_EFFECT_CUE_VIEW* const pEffectOverride,
	const VALTAN_PATTERN_SOUND_CUE* const pSoundOverride,
	std::string& strOutStatus) const
{
	if (VALTAN_VIEW_ADMISSION::ADMITTED != m_eAdmission ||
		nullptr == m_pAnimationTool)
	{
		strOutStatus =
			"Sequencer timing edit requires a fresh FULL_JOIN admission; stale-preserved rows are display-only.";
		return false;
	}
	if (!m_bPatternShakesReady)
	{
		strOutStatus =
			"Sequencer timing edit is blocked because the Pattern Shake dependency source is not admitted: " +
			m_strShakeStatus;
		return false;
	}

	bool_t bPatternSoundDirty = false;
	std::string PatternSoundStatus;
	const VALTAN_PATTERN_SOUND_CUE_DOCUMENT* const pPatternSounds =
		m_pAnimationTool->Get_ValtanCompositionPatternSoundDraft(
			bPatternSoundDirty, PatternSoundStatus);
	if (nullptr == pPatternSounds)
	{
		strOutStatus =
			"Sequencer timing edit is blocked because the Pattern Sound dependency owner is not admitted: " +
			PatternSoundStatus;
		return false;
	}

	std::size_t iEffectOverrideMatches = 0u;
	for (const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Existing : Stage.ProductCues)
	{
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW* pCandidate = &Existing;
		if (nullptr != pEffectOverride &&
			Existing.strBindingId == pEffectOverride->strBindingId &&
			Existing.strOccurrenceId == pEffectOverride->strOccurrenceId)
		{
			pCandidate = pEffectOverride;
			++iEffectOverrideMatches;
		}
		if (pCandidate->strPatternId != Pattern.strPatternId ||
			pCandidate->strStageId != Stage.strStageId ||
			pCandidate->strActionId != Stage.strActionId)
		{
			strOutStatus =
				"Effect dependency no longer resolves the exact Pattern/Stage/action tuple: " +
				pCandidate->strOccurrenceId + ".";
			return false;
		}
		if (pCandidate->bUsesStageClock)
		{
			if (!ValidateValtanEffectInvocationClock(m_pAnimationTool, Stage, *pCandidate, strOutStatus)) return false;
			continue;
		}
		if (!ValidateClipSourceDependencyWindow(
				Stage, pCandidate->strClipOccurrenceId,
				pCandidate->iSourceStartMs, pCandidate->bHasSourceEnd,
				pCandidate->iSourceEndMs,
				"each_loop" == pCandidate->strRepeatPolicy,
				"Effect occurrence " + pCandidate->strOccurrenceId,
				strOutStatus, pCandidate->strRepeatPolicy == "once"))
		{
			return false;
		}
	}
	if (nullptr != pEffectOverride && 1u != iEffectOverrideMatches)
	{
		strOutStatus =
			"Sequencer Effect edit lost its exact bindingId/occurrenceId predecessor.";
		return false;
	}

	std::size_t iSoundOverrideMatches = 0u;
	for (const VALTAN_PATTERN_SOUND_CUE& Existing : pPatternSounds->Cues)
	{
		if (Existing.strPatternId != Pattern.strPatternId ||
			Existing.strStageId != Stage.strStageId)
		{
			continue;
		}
		const VALTAN_PATTERN_SOUND_CUE* pCandidate = &Existing;
		if (nullptr != pSoundOverride &&
			Existing.strBindingId == pSoundOverride->strBindingId &&
			Existing.strOccurrenceId == pSoundOverride->strOccurrenceId)
		{
			pCandidate = pSoundOverride;
			++iSoundOverrideMatches;
		}
		if (pCandidate->strActionId != Stage.strActionId ||
			!ValidateClipSourceDependencyWindow(
				Stage, pCandidate->strClipOccurrenceId,
				pCandidate->iStartMs, false, 0u,
				VALTAN_PATTERN_SOUND_REPEAT_POLICY::EACH_LOOP ==
					pCandidate->eRepeatPolicy,
				"Pattern Sound occurrence " + pCandidate->strOccurrenceId,
				strOutStatus))
		{
			if (strOutStatus.empty())
			{
				strOutStatus =
					"Pattern Sound dependency no longer resolves the exact Pattern/Stage/action tuple: " +
					pCandidate->strOccurrenceId + ".";
			}
			return false;
		}
	}
	if (nullptr != pSoundOverride && 1u != iSoundOverrideMatches)
	{
		strOutStatus =
			"Sequencer Sound edit lost its exact bindingId/occurrenceId predecessor.";
		return false;
	}

	for (const VALTAN_PATTERN_SHAKE_CUE& Cue : m_PatternShakes.Cues)
	{
		if (Cue.strPatternId != Pattern.strPatternId ||
			Cue.strStageId != Stage.strStageId)
		{
			continue;
		}
		if (Cue.strActionId != Stage.strActionId ||
			!ValidateClipSourceDependencyWindow(
				Stage, Cue.strClipOccurrenceId, Cue.iStartMs, false, 0u,
				VALTAN_PATTERN_SHAKE_REPEAT_POLICY::EACH_LOOP ==
					Cue.eRepeatPolicy,
				"Pattern Shake occurrence " + Cue.strOccurrenceId,
				strOutStatus))
		{
			if (strOutStatus.empty())
			{
				strOutStatus =
					"Pattern Shake dependency no longer resolves the exact Pattern/Stage/action tuple: " +
					Cue.strOccurrenceId + ".";
			}
			return false;
		}
	}

	strOutStatus = bPatternSoundDirty ?
		"Validated Effect, unsaved Pattern Sound draft, and Pattern Shake windows." :
		"Validated Effect, Pattern Sound, and Pattern Shake dependency windows.";
	return true;
}

bool_t Client::CValtanActionWorkbench::Apply_EffectOccurrenceTiming(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Current,
	const uint32_t iSourceStartMs,
	const uint32_t iSourceEndMs,
	std::string& strOutStatus)
{
	if (VALTAN_VIEW_ADMISSION::ADMITTED != m_eAdmission ||
		nullptr == m_pBalanceTool)
	{
		strOutStatus =
			"Sequencer Effect timing requires one admitted source occurrence.";
		return false;
	}
	VALTAN_PRODUCT_EFFECT_CUE_VIEW Candidate = Current;
	Candidate.iSourceStartMs = iSourceStartMs;
	if (Candidate.bUsesStageClock) Candidate.iStageOffsetMs = iSourceStartMs;
	Candidate.bHasSourceEnd = iSourceEndMs != 0u;
	Candidate.iSourceEndMs = iSourceEndMs;
	Candidate.strStopPolicy = Candidate.bHasSourceEnd ? "cue_end" : "natural";
	Candidate.eStopPolicy = Candidate.bHasSourceEnd ?
		EFFECT_STOP_POLICY::CUE_END : EFFECT_STOP_POLICY::NATURAL;
	if (Candidate.iSourceStartMs == Current.iSourceStartMs &&
		Candidate.iSourceEndMs == Current.iSourceEndMs &&
		Candidate.bHasSourceEnd == Current.bHasSourceEnd)
	{
		strOutStatus = "Sequencer Effect timing draft is unchanged.";
		return true;
	}
	if (!ValidateValtanEffectInvocationClock(m_pAnimationTool, Stage, Candidate, strOutStatus) ||
		!Validate_TimelineDependencyWindows(
			Pattern, Stage, &Candidate, nullptr, strOutStatus))
	{
		strOutStatus =
			"Sequencer Effect timing rejected before the typed draft changed: " +
			strOutStatus;
		return false;
	}
	if (!m_pBalanceTool->Update_ValtanStageEffectCue(
			Pattern.strPatternId, Stage.strStageId, Stage.strActionId,
			Current.strBindingId, Current.strOccurrenceId,
			Candidate, strOutStatus))
	{
		return false;
	}
	m_strEffectEditIdentity.clear();
	strOutStatus +=
		" Timeline edit remains unsaved; use Save when the timing is ready.";
	return true;
}

bool_t Client::CValtanActionWorkbench::Apply_SoundTimelinePlacement(
	const VALTAN_PATTERN_VIEW& Pattern, const VALTAN_STAGE_VIEW& SourceStage,
	const VALTAN_PATTERN_SOUND_CUE& Current, const uint32_t iPatternStartMs,
	const uint32_t iPlaybackOffsetMs, const std::optional<uint32_t> PlaybackDurationMs,
	std::string& strOutStatus, const bool_t bRefreshPreview)
{
	if (!m_pAnimationTool || !Can_MutateValtanView(m_eAdmission))
	{ strOutStatus = "Sound editing requires the admitted source draft."; return false; }
	const TIMELINE_ITEM* target = nullptr;
	for (const auto& item : m_TimelineItems)
		if (item.eLane == TIMELINE_LANE::ANIMATION && item.strPatternId == Pattern.strPatternId &&
			iPatternStartMs >= item.iStartMs && iPatternStartMs < item.iEndMs &&
			(!target || item.iStartMs >= target->iStartMs)) target = &item;
	if (!target)
	{ strOutStatus = "Place the Sound start inside an Animation box; the existing Sound was preserved."; return false; }
	const auto stage = std::find_if(Pattern.Stages.begin(), Pattern.Stages.end(),
		[&](const auto& row) { return row.strStageId == target->strStageId; });
	if (stage == Pattern.Stages.end()) return false;
	const auto clip = std::find_if(stage->ClipOccurrences.begin(), stage->ClipOccurrences.end(),
		[&](const auto& row) { return row.strClipOccurrenceId == target->strStableId; });
	if (clip == stage->ClipOccurrences.end() || !std::isfinite(clip->fPlayRate) || clip->fPlayRate <= 0.f)
	{ strOutStatus = "The destination Sound clip has no valid playback rate."; return false; }
	const uint64_t sourceMs = clip->iSourceStartMs + static_cast<uint64_t>(std::llround(
		(iPatternStartMs - target->iStartMs) * double(clip->fPlayRate)));
	if (sourceMs > 600000u) return false;
	if (!m_pAnimationTool->Patch_ValtanCompositionPatternSoundPlacement(Pattern, SourceStage, *stage,
		Current.strOccurrenceId, clip->strClipOccurrenceId, static_cast<uint32_t>(sourceMs),
		iPlaybackOffsetMs, PlaybackDurationMs, Current.eRepeatPolicy, strOutStatus)) return false;
	m_strSelectedStageId = stage->strStageId;
	m_strSelectedStableId = Current.strOccurrenceId;
	m_eDetailOwner = DETAIL_OWNER::SOUND;
	Invalidate_TimelineCache();
	if (bRefreshPreview) Refresh_PatternLocalPreviewAfterMutation(&Pattern, strOutStatus);
	return true;
}

bool_t Client::CValtanActionWorkbench::
Apply_PatternSoundOccurrenceTiming(
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage,
	const VALTAN_PATTERN_SOUND_CUE& Current,
	const uint32_t iSourceStartMs,
	std::string& strOutStatus)
{
	if (VALTAN_VIEW_ADMISSION::ADMITTED != m_eAdmission ||
		nullptr == m_pAnimationTool)
	{
		strOutStatus =
			"Sequencer Sound timing requires a FULL_JOIN admission and the shared Composition draft.";
		return false;
	}
	VALTAN_PATTERN_SOUND_CUE Candidate = Current;
	Candidate.iStartMs = iSourceStartMs;
	if (Candidate.iStartMs == Current.iStartMs)
	{
		strOutStatus = "Sequencer Pattern Sound timing draft is unchanged.";
		return true;
	}
	if (!Validate_TimelineDependencyWindows(
			Pattern, Stage, nullptr, &Candidate, strOutStatus))
	{
		strOutStatus =
			"Sequencer Sound timing rejected before the typed owner draft changed: " +
			strOutStatus;
		return false;
	}
	if (!m_pAnimationTool->Patch_ValtanCompositionPatternSound(
			Pattern, Stage, Current.strOccurrenceId, Current.strSoundEvent,
			iSourceStartMs, Current.eRepeatPolicy, strOutStatus))
	{
		return false;
	}
	strOutStatus +=
		" Timeline edit remains an unsaved Pattern Sound owner draft; Save/Apply was not invoked.";
	return true;
}

void Client::CValtanActionWorkbench::Render_SelectedStageGapControl(
	const VALTAN_PATTERN_VIEW& Pattern,
	const bool_t bPatternMutationAdmitted)
{
	const VALTAN_STAGE_VIEW* const pStage = Find_SelectedStage(&Pattern);
	if (nullptr == pStage || nullptr == m_pBalanceTool ||
		VALTAN_VIEW_ADMISSION::ADMITTED != m_eAdmission)
	{
		return;
	}

	CBalanceTool::PATTERN_STAGE_EDIT Draft;
	std::string Status;
	uint32_t iAnimationWallMs = 0u;
	if (!m_pBalanceTool->Get_ValtanStageDraft(
			Pattern.strPatternId, pStage->strStageId, Draft, Status) ||
		!Draft.durationEditable || !Draft.animationEditable ||
		"LOOP_TO_STAGE_END" == Draft.animationEndPolicy ||
		!ComputeExactAnimationWallMs(Draft, iAnimationWallMs))
	{
		return;
	}

	int32_t iGapMs = static_cast<int32_t>(
		Draft.durationMs > iAnimationWallMs ?
			Draft.durationMs - iAnimationWallMs : 0u);
	ImGui::PushID("##SelectedStageSequencerGap");
	ImGui::SetNextItemWidth(220.f);
	ImGui::BeginDisabled(!bPatternMutationAdmitted);
	(void)ImGui::InputInt(
		"Selected Stage Gap (ms)", &iGapMs, 10, 100);
	if (ImGui::IsItemDeactivatedAfterEdit())
	{
		iGapMs = (std::clamp)(iGapMs, 0, 120000);
		const uint64_t iDurationMs =
			static_cast<uint64_t>(iAnimationWallMs) +
			static_cast<uint32_t>(iGapMs);
		if (iDurationMs <= 600000u)
		{
			Draft.durationMs = static_cast<uint32_t>(iDurationMs);
			Draft.animationEndPolicy = 0 == iGapMs ?
				"EXACT" : "HOLD_LAST_POSE";
			if (SetValtanStageDraftWithSoundDependencyAdmission(
					m_pAnimationTool, m_pBalanceTool,
					m_bPatternShakesReady ? &m_PatternShakes : nullptr,
					Pattern, *pStage, Draft, Status))
			{
				Invalidate_TimelineCache();
			}
			m_strStatus = std::move(Status);
		}
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled(
		"%s | Animation %u ms -> Stage %u ms | Save when ready",
		pStage->strStageId.c_str(), iAnimationWallMs, Draft.durationMs);
	ImGui::PopID();
}

void Client::CValtanActionWorkbench::Render_Timeline(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const bool_t bLocalPreviewAdmitted,
	const bool_t bMutationAdmitted,
	const bool_t bPatternMutationAdmitted)
{
	const auto metrics = CompositionTimeline::GetCompactRowMetrics();
	const float TIMELINE_ROW_HEIGHT = metrics.laneHeight;
	const float TIMELINE_BLOCK_HEIGHT = metrics.boxHeight;
    if ((m_bTimelineMoveActive || m_bTimelineTrimActive) &&
        (ImGui::IsKeyPressed(ImGuiKey_Escape) || !m_pBalanceTool ||
         m_iTimelineGestureDraftGeneration != m_pBalanceTool->Get_ValtanDraftGeneration() ||
         m_iTimelineGestureSoundGeneration != (m_pAnimationTool ? m_pAnimationTool->Get_ValtanCompositionPatternSoundDraftGeneration() : 0u) ||
         m_iTimelineGestureEffectRevision != CEffectV2Catalog::Get().Get_Revision()))
    {
        m_bTimelineMoveActive = m_bTimelineTrimActive = false;
        m_strStatus = "Timeline gesture canceled because Escape was pressed or its source draft changed; timing preserved.";
    }
	const bool_t bHasUnsavedChanges = m_bAuthoringDraftDirty || m_bPatternSoundDependencyDirty || m_CombatObjectSounds.bDraftDirty ||
		CEffectV2Catalog::Get().Has_BossValtanBindingDraft() || Has_PendingSelectedEffectDetails(pPattern);
	const bool_t bSaveJobBlocking = m_pBalanceTool &&
		(m_pBalanceTool->Is_ValtanSaveJobBlockingAuthoring() || m_pBalanceTool->Is_ServerRuntimeSetPublishRunning());
	ImGui::BeginDisabled(!pPattern || !bPatternMutationAdmitted || !bHasUnsavedChanges || bSaveJobBlocking);
	if (ImGui::Button("Save##CompositionSequencer"))
	{
		m_bPublishWithPendingSave = false;
		m_bSavePatternRequested = true;
	}
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("Save current edits to source. Published runtime files and the active Server revision are shown below.");
    const bool keyboard = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::GetIO().WantTextInput;
    if (keyboard && ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false) &&
        pPattern && bPatternMutationAdmitted && bHasUnsavedChanges && !bSaveJobBlocking)
    {
        m_bPublishWithPendingSave = false;
        m_bSavePatternRequested = true;
    }
	ImGui::EndDisabled();

	CAnimation_Tool::COMPOSITION_PREVIEW_STATE Preview;
	if (m_pAnimationTool)
	{
		m_pAnimationTool->Set_ValtanCompositionLoop(m_bLoopPreview);
		Preview = m_pAnimationTool->Get_ValtanCompositionPreviewState();
	}
	const bool sourcePreview = Preview.bSourceSequencePlaying;
	const bool serverPlayback = IsValtanServerPlaybackBusy(m_pValtanBossTool);
	const bool matchingPreview = Preview.bPlaying &&
		(sourcePreview || (pPattern && Preview.strPatternId == pPattern->strPatternId));
	if (matchingPreview) m_iPlayheadMs = Preview.iPositionMs;
	const uint32_t duration = sourcePreview ? Preview.iDurationMs : m_iTimelineDurationMs;
	const bool canPreview = m_pAnimationTool && !serverPlayback &&
		(sourcePreview || (pPattern && bLocalPreviewAdmitted));
	ImGui::SameLine();
	ImGui::BeginDisabled(!canPreview);
	if (ImGui::Button("Play Preview"))
	{
		const uint32_t cursor = m_iPlayheadMs >= duration ? 0u : m_iPlayheadMs;
		if (sourcePreview)
		{
			m_BoneEditor.Stop();
			(void)m_pAnimationTool->Seek_ValtanCompositionSourceSequence(cursor, false, m_strStatus);
		}
		else (void)Seek_EffectivePreview(*pPattern, cursor, false, m_strStatus);
	}
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("Preview the current draft from the cursor using the selected Server Stage path. Pause and seek inspect presentation; damage and live player decisions require Play Pattern.");
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!canPreview || !matchingPreview);
	if (ImGui::Button(Preview.bPaused ? "Resume" : "Pause"))
	{
		const uint32_t cursor = Preview.bPaused && m_iPlayheadMs >= duration ? 0u : m_iPlayheadMs;
		if (sourcePreview) (void)m_pAnimationTool->Seek_ValtanCompositionSourceSequence(cursor, !Preview.bPaused, m_strStatus);
		else (void)Seek_EffectivePreview(*pPattern, cursor, !Preview.bPaused, m_strStatus);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!m_pAnimationTool || serverPlayback);
	if (ImGui::Button("Reset"))
	{
		m_PendingLocalPreview.reset();
		m_pAnimationTool->Stop_ValtanCompositionPattern(m_strStatus);
		m_iPlayheadMs = 0u;
		m_iPreviewDraftGeneration = 0u;
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::BeginDisabled(!pPattern || !bPatternMutationAdmitted || !m_bProductSourceReady || bSaveJobBlocking || bHasUnsavedChanges || serverPlayback);
	if (ImGui::Button("Play Pattern"))
		(void)Play_ServerVerification(*pPattern, m_strStatus);
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("Run the saved, published Pattern on the Server. Save/Publish and the active Server revision must agree; no older Pattern is substituted for this draft.");
	ImGui::EndDisabled();
	ImGui::SameLine();
	Render_EditHistoryButtons("TimelineHistory");
	ImGui::SameLine();
	ImGui::Checkbox("Loop", &m_bLoopPreview);
	ImGui::SameLine();
	ImGui::TextDisabled("%s | %.3f / %.3f s%s%s", bHasUnsavedChanges ? "Unsaved source edits" : "Source saved",
		m_iPlayheadMs * 0.001, duration * 0.001, matchingPreview && Preview.bPaused ? " (paused)" : "",
		serverPlayback ? " | Server Pattern" : sourcePreview ? " | source clip" : "");
	const float publishWidth = ImGui::CalcTextSize("Save & Publish").x + ImGui::GetStyle().FramePadding.x * 2.f;
	const float publishAfterStatus = ImGui::GetItemRectMax().x - ImGui::GetWindowPos().x + ImGui::GetStyle().ItemSpacing.x;
	ImGui::SameLine((std::max)(publishAfterStatus, ImGui::GetWindowContentRegionMax().x - publishWidth));
	ImGui::BeginDisabled(!pPattern || !bPatternMutationAdmitted || bSaveJobBlocking);
	if (ImGui::Button("Save & Publish##CompositionSequencer"))
	{
		m_bPublishWithPendingSave = true;
		m_bSavePatternRequested = true;
	}
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("Save current edits, validate and publish that exact revision. Already saved source can also be published here. Server activation still requires the matching runtime revision.");
	ImGui::EndDisabled();
	const bool_t bPublishedSource = !bHasUnsavedChanges &&
		(m_bProductSourceReady ||
		 (POST_SAVE_STATE::PUBLISHED_RESTART_REQUIRED == m_ePostSaveState &&
		  m_strPostSaveRevision == m_strPinnedCanonicalSourceRevision));
	LostArk::Shared::GameplayDataRevision ActiveRevision{};
	std::string ActiveRevisionStatus;
	const bool_t bServerRevisionObserved = m_pValtanBossTool &&
		m_pValtanBossTool->Observe_ServerActivePatternRevision(ActiveRevision, ActiveRevisionStatus);
	const bool_t bPublishJobRunning = (0u != m_iPendingSaveJobId && m_bPendingSavePublishesRuntime) ||
		(m_pBalanceTool && m_pBalanceTool->Is_ServerRuntimeSetPublishRunning());
	const char* publishState = bPublishJobRunning ? "running" :
		POST_SAVE_STATE::PUBLISH_FAILED == m_ePostSaveState || POST_SAVE_STATE::APPLY_FAILED == m_ePostSaveState ? "failed" :
		bPublishedSource ? "published" : bHasUnsavedChanges ? "pending after save" : "pending";
	ImGui::TextWrapped("Source: %s | Publish: %s | Server Play: %s",
		bHasUnsavedChanges ? "unsaved edits" : VALTAN_VIEW_ADMISSION::ADMITTED == m_eAdmission ? "saved" : "not admitted",
		publishState, bServerRevisionObserved ? "connected; revision checked on Play" :
			!bPublishedSource && VALTAN_VIEW_ADMISSION::ADMITTED == m_eAdmission ?
			"waiting for published source; Source Save and local Preview are available" :
			ActiveRevisionStatus.empty() ? "unavailable" : ActiveRevisionStatus.c_str());
	if (sourcePreview)
		ImGui::TextDisabled("Previewing a source animation. Open its owning Pattern from Resources to view and edit the complete effect sequence.");
	if (ImGui::TreeNode("Save / Publish details"))
	{
		Render_PostSaveState();
		if (!m_strStatus.empty()) ImGui::TextWrapped("%s", m_strStatus.c_str());
		if (!m_bProductSourceReady) ImGui::TextWrapped("Publish required: %s", m_strProductReadiness.c_str());
		ImGui::TreePop();
	}
	if (!m_strCinematicPreviewStatus.empty())
		ImGui::TextWrapped("%s", m_strCinematicPreviewStatus.c_str());
	if (!m_strStatus.empty()) ImGui::TextWrapped("%s", m_strStatus.c_str());
	int32_t iPreviewPath = static_cast<int32_t>(m_ePreviewPath);
	ImGui::SetNextItemWidth(250.f);
	if (ImGui::Combo(
			"Preview Branch", &iPreviewPath,
			"Normal / Timeout\0Counter Hit -> Groggy\0Wall Contact -> Groggy\0Wall Contact -> Part Break\0Capture Success -> Throw\0"))
	{
		if (Preview.bPlaying && nullptr != m_pAnimationTool)
		{
			std::string StopStatus;
			m_pAnimationTool->Stop_ValtanCompositionPattern(StopStatus);
		}
		m_ePreviewPath = static_cast<VALTAN_PATTERN_PREVIEW_PATH>(iPreviewPath);
		m_iPreviewDraftGeneration = 0u;
		m_iPlayheadMs = 0u;
		Invalidate_TimelineCache();
	}
	ImGui::SameLine();
	ImGui::TextDisabled("%s", PreviewPathLabel(m_ePreviewPath));
	const uint32_t iFitDurationMs = (std::min)(
		m_iTimelineDurationMs, MAX_TIMELINE_RENDER_DURATION_MS);
	const float fFitClockWidth = (std::max)(
		1.f, ImGui::GetContentRegionAvail().x -
			TIMELINE_LANE_LABEL_WIDTH - TIMELINE_CANVAS_END_PADDING_PX -
			ImGui::GetStyle().ScrollbarSize -
			ImGui::GetStyle().ChildBorderSize * 2.f);
	ImGui::SetNextItemWidth(220.f);
	if (ImGui::SliderFloat(
			"Zoom (px/sec)", &m_fTimelinePixelsPerSecond,
			1.f, 500.f, "%.0f", ImGuiSliderFlags_Logarithmic))
	{
		m_fTimelinePixelsPerSecond = (std::clamp)(
			m_fTimelinePixelsPerSecond, 1.f, 500.f);
		Pack_TimelineSubrows();
	}
	ImGui::SameLine();
	ImGui::BeginDisabled(0u == m_iTimelineDurationMs);
	if (ImGui::Button("Fit Timeline") && 0u < iFitDurationMs)
	{
		m_fTimelinePixelsPerSecond = (std::clamp)(
			fFitClockWidth * 1000.f /
				static_cast<float>(iFitDurationMs),
			1.f, 500.f);
		Pack_TimelineSubrows();
	}
	ImGui::EndDisabled();
	if (!m_bSharedPaneRendering)
	{
		ImGui::SameLine();
		if (ImGui::Button(
			m_bTimelineMaximized ? "Restore Timeline" : "Maximize Timeline"))
		{
			if (m_bTimelineMaximized)
			{
				m_bTimelineMaximized = false;
			}
			else
			{
				m_bTimelineMaximized = true;
			}
		}
	}
	if (nullptr == pPattern)
	{
		ImGui::TextDisabled("Select one Pattern to build its joined tracks.");
		return;
	}
	const VALTAN_STAGE_VIEW* const pSelectedStage = Find_SelectedStage(pPattern);
	ImGui::BeginDisabled(
		nullptr == pSelectedStage || nullptr == m_pAnimationTool ||
		!bLocalPreviewAdmitted);
	if (ImGui::Button("Play Selected Stage (All Slots)") &&
		nullptr != pSelectedStage)
	{
		const std::array<VALTAN_PATTERN_PREVIEW_PATH, 6u> CandidatePaths = {
			m_ePreviewPath,
			VALTAN_PATTERN_PREVIEW_PATH::NORMAL,
			VALTAN_PATTERN_PREVIEW_PATH::COUNTER_GROGGY,
			VALTAN_PATTERN_PREVIEW_PATH::WALL_GROGGY,
			VALTAN_PATTERN_PREVIEW_PATH::PART_BREAK,
			VALTAN_PATTERN_PREVIEW_PATH::CAPTURE_SUCCESS,
		};
		bool_t bFoundPath = false;
		uint32_t iSelectedStageStartMs = 0u;
		std::set<VALTAN_PATTERN_PREVIEW_PATH> TriedPaths;
		std::string Status;
		for (const VALTAN_PATTERN_PREVIEW_PATH eCandidatePath : CandidatePaths)
		{
			if (!TriedPaths.insert(eCandidatePath).second)
				continue;
			std::vector<const VALTAN_STAGE_VIEW*> CandidateStages;
			if (!CValtanPatternTree::Build_PreviewStagePath(
					*pPattern, eCandidatePath, CandidateStages, Status))
			{
				continue;
			}
			uint64_t iStageStartMs = 0u;
			for (const VALTAN_STAGE_VIEW* const pCandidateStage :
				CandidateStages)
			{
				if (nullptr == pCandidateStage)
					continue;
				if (pCandidateStage->strActionId ==
					pSelectedStage->strActionId)
				{
					m_ePreviewPath = eCandidatePath;
					iSelectedStageStartMs = static_cast<uint32_t>(
						SaturatingU32(iStageStartMs));
					bFoundPath = true;
					break;
				}
				iStageStartMs += pCandidateStage->iDurationMs;
			}
			if (bFoundPath)
				break;
		}
		if (bFoundPath)
		{
			m_iPreviewDraftGeneration = 0u;
			Invalidate_TimelineCache();
			m_iPlayheadMs = iSelectedStageStartMs;
			(void)Seek_EffectivePreview(
				*pPattern, iSelectedStageStartMs, false, Status);
		}
		else
		{
			Status =
				"Selected Stage is not reachable on any admitted preview branch.";
		}
		m_strStatus = std::move(Status);
	}
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled(
		"Starts this Stage on the Arena Clone with Animation, Effect and collider mirror joined on one clock.");
	if (ImGui::CollapsingHeader("Animation resource to Stage"))
	{
	const auto SelectedSequence = std::find_if(
		m_AnimationSequences.begin(), m_AnimationSequences.end(),
		[this](const CAnimation_Tool::COMPOSITION_SEQUENCE_VIEW& Sequence)
		{
			return Sequence.strStableId == m_strSelectedSequenceStableId;
		});
	ImGui::SeparatorText("Selected Sequence -> Selected Stage");
	if (SelectedSequence == m_AnimationSequences.end())
	{
		ImGui::TextDisabled(
			"Choose one Animation resource on the left; click an Animation block below to choose its Stage.");
	}
	else
	{
		ImGui::Text("%s | %zu clips",
			SelectedSequence->strDisplayName.c_str(),
			SelectedSequence->Clips.size());
		ImGui::SameLine();
		ImGui::TextDisabled("-> %s",
			nullptr == pSelectedStage ? "select a Stage" :
				pSelectedStage->strStageId.c_str());
		ImGui::BeginDisabled(nullptr == m_pAnimationTool);
		if (ImGui::Button("Preview Candidate on Arena Clone"))
		{
			std::string Status;
			if (m_pAnimationTool->Preview_ValtanCompositionSequence(
					SelectedSequence->iSkillId,
					SelectedSequence->iSequenceIndex, Status))
			{
				m_bPreviewOwnerClaimRequested = true;
			}
			m_strStatus = std::move(Status);
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(
			nullptr == pSelectedStage || !bPatternMutationAdmitted);
		if (ImGui::Button("Replace Stage Slots"))
			(void)Apply_SelectedSequenceToStage(
				*pPattern, *pSelectedStage, false);
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(!bPatternMutationAdmitted);
		if (ImGui::Button("Append to Pattern End"))
		{
			(void)Append_SelectedSequenceToPattern(*pPattern);
			ImGui::EndDisabled();
			return;
		}
		ImGui::EndDisabled();
		ImGui::SameLine();
		ImGui::BeginDisabled(nullptr == m_pAnimationTool ||
			!bPatternMutationAdmitted);
		if (ImGui::Button("Use for Create New Pattern"))
		{
			std::string Status;
			if (m_pAnimationTool->Stage_ValtanCompositionIntakeSequence(
					SelectedSequence->iSkillId,
					SelectedSequence->iSequenceIndex,
					nullptr == pPattern ? std::string{} :
						pPattern->strPatternId,
					nullptr == pSelectedStage ? std::string{} :
						pSelectedStage->strStageId,
					Status))
			{
				m_bPatternsWindowVisible = true;
				m_iRequestedPatternTab = 1;
			}
			m_strStatus = std::move(Status);
		}
		ImGui::EndDisabled();
	}
	}
	Prune_TimelineSelection();
	const auto selectedBoxIt = std::find_if(
		m_TimelineItems.begin(), m_TimelineItems.end(),
		[this, pPattern, pSelectedStage](const TIMELINE_ITEM& Item)
		{
			return nullptr != pSelectedStage &&
				Item.strPatternId == pPattern->strPatternId &&
				Item.strStageId == pSelectedStage->strStageId &&
				Item.eOwner == m_eDetailOwner &&
				Item.strStableId == m_strSelectedStableId &&
(Item.eLane != TIMELINE_LANE::STAGE);
		});
	// 버튼 callback에서 preview를 다시 준비하면 timeline cache가 재생성될 수 있다.
	const std::optional<TIMELINE_ITEM> SelectedTimelineBox = selectedBoxIt != m_TimelineItems.end() ?
		std::optional<TIMELINE_ITEM>{*selectedBoxIt} : std::nullopt;
	const bool_t bTimelineBoxSelected = SelectedTimelineBox.has_value();
	const bool_t bGroupSelection = m_TimelineSelection.size() > 1u;
	const bool_t bSelectedSoundBox = bTimelineBoxSelected &&
		DETAIL_OWNER::SOUND == SelectedTimelineBox->eOwner;
	const bool_t bSelectedV2Box = bTimelineBoxSelected &&
		SelectedTimelineBox->bEffectV2Binding;
	const bool_t bSelectedLogicBox = bTimelineBoxSelected &&
		DETAIL_OWNER::GAMEPLAY_STAGE == SelectedTimelineBox->eOwner &&
		TIMELINE_LANE::LOGIC == SelectedTimelineBox->eLane;
	const bool_t bSelectedColliderBox = bTimelineBoxSelected &&
		DETAIL_OWNER::GAMEPLAY_STAGE == SelectedTimelineBox->eOwner &&
		TIMELINE_LANE::COLLIDER == SelectedTimelineBox->eLane;
	const bool_t bSelectedStatusBox = bSelectedLogicBox &&
		std::string::npos != SelectedTimelineBox->strStableId.find(
			"/action/SET_");
	const bool_t bSelectedReadOnlyBox = bTimelineBoxSelected && !SelectedTimelineBox->bEditable;
	const bool_t bSelectedDuplicateOwner = bTimelineBoxSelected &&
		(SelectedTimelineBox->eOwner == DETAIL_OWNER::ANIMATION ||
		 SelectedTimelineBox->eOwner == DETAIL_OWNER::EFFECT ||
		 SelectedTimelineBox->eOwner == DETAIL_OWNER::SOUND ||
		 (SelectedTimelineBox->eLane == TIMELINE_LANE::COLLIDER &&
		  SelectedTimelineBox->strStableId == SelectedTimelineBox->strStageId + "/collider"));
	const bool_t bSelectedBoxMutationAdmitted = bTimelineBoxSelected &&
		!bSelectedReadOnlyBox &&
		(bSelectedV2Box ? bMutationAdmitted : (bSelectedSoundBox ?
			bMutationAdmitted :
			bPatternMutationAdmitted));
	ImGui::SeparatorText("Selected Box");
	const bool_t bCanReorderSelection = m_TimelineSelection.empty() ?
		(m_eDetailOwner == DETAIL_OWNER::ANIMATION ||
		 (m_eDetailOwner == DETAIL_OWNER::GAMEPLAY_STAGE && pSelectedStage && m_strSelectedStableId == pSelectedStage->strStageId)) :
		std::all_of(m_TimelineSelection.begin(), m_TimelineSelection.end(), [](const auto& key) {
			return key.eOwner == DETAIL_OWNER::ANIMATION ||
				(key.eOwner == DETAIL_OWNER::GAMEPLAY_STAGE && key.strStableId == key.strStageId);
		});
	ImGui::BeginDisabled(!bCanReorderSelection || !bPatternMutationAdmitted ||
		!pPattern->bManualServerAudition || bSaveJobBlocking);
	int32_t reorderDirection = 0;
	if (ImGui::Button("< Earlier##ValtanSequencerSelection")) reorderDirection = -1;
	ImGui::SameLine();
	if (ImGui::Button("Later >##ValtanSequencerSelection")) reorderDirection = 1;
	ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::TextDisabled("Move selected Stages and their Animation together by one slot.");
	if (reorderDirection != 0)
	{
		(void)Reorder_SelectedStages(*pPattern, reorderDirection, m_strStatus);
		return; // 소유 도구 transaction과 preview 갱신이 끝난 뒤 view를 다시 조회한다.
	}
	ImGui::TextDisabled("Drag empty track space to select boxes. Shift adds to selection; Esc cancels. Drag a selected box to move it.");
	if (bGroupSelection)
	{
		ImGui::Text("%zu boxes selected", m_TimelineSelection.size());
		ImGui::TextDisabled("Move, Copy, Paste, Duplicate and Delete apply to the selected boxes together. Animation moves snap to Sequence boundaries.");
		ImGui::BeginDisabled(!bPatternMutationAdmitted || bSaveJobBlocking);
		ImGui::SetNextItemWidth(130.f);
		ImGui::InputInt("Move selection (ms)", &m_iTimelineGroupMoveMs);
		ImGui::SameLine();
		if (ImGui::Button("Move Selected"))
			(void)Apply_TimelineGroupEdit(*pPattern, TIMELINE_GROUP_EDIT::MOVE_BOXES, m_iTimelineGroupMoveMs, m_strStatus);
		ImGui::SameLine();
		if (ImGui::Button("Duplicate Selected"))
			(void)Execute_CompositionEdit(COMPOSITION_EDIT_COMMAND::DUPLICATE_SELECTION, m_strStatus);
		ImGui::SameLine();
		if (ImGui::Button("Delete Selected") || (keyboard && bPatternMutationAdmitted && !bSaveJobBlocking &&
			ImGui::IsKeyPressed(ImGuiKey_Delete, false)))
		{
			const bool hasStage = std::any_of(m_TimelineSelection.begin(), m_TimelineSelection.end(),
				[](const auto& key) { return key.eOwner == DETAIL_OWNER::GAMEPLAY_STAGE && key.strStableId == key.strStageId; });
			const bool changed = hasStage ? Delete_SelectedStages(*pPattern, m_strStatus) :
				Apply_TimelineGroupEdit(*pPattern, TIMELINE_GROUP_EDIT::DELETE_BOXES, 0, m_strStatus);
			ImGui::EndDisabled();
			if (changed) return;
		}
		else
			ImGui::EndDisabled();
	}

	if (bTimelineBoxSelected)
	{
		ImGui::TextDisabled("%s | %s",
			Owner_Label(SelectedTimelineBox->eOwner),
			SelectedTimelineBox->strLabel.c_str());
		if (bSelectedReadOnlyBox)
		{
			ImGui::TextDisabled(
				"This row follows another clock. Select its owning box in Box Detail to edit that timing.");
		}
	}
	else
	{
		ImGui::TextDisabled(
			"Select a box to open Box Detail. Use the track + button to add a resource.");
	}
	if (bSelectedDuplicateOwner && !bGroupSelection)
	{
		ImGui::SameLine();
		ImGui::BeginDisabled(!bSelectedBoxMutationAdmitted);
		if (ImGui::Button("Duplicate Box"))
		{
			std::string Status;
			(void)Duplicate_SelectedTimelineBox(
				*pPattern, *pSelectedStage, Status);
			m_strStatus = std::move(Status);
		}
		ImGui::EndDisabled();
	}
	if (bSelectedDuplicateOwner && !bGroupSelection)
	{
		ImGui::SameLine();
		ImGui::BeginDisabled(!bSelectedBoxMutationAdmitted);
		if (ImGui::Button("Delete Box") || (keyboard && bSelectedBoxMutationAdmitted && ImGui::IsKeyPressed(ImGuiKey_Delete, false)))
		{
			std::string Status;
			const bool changed = Delete_SelectedTimelineBox(
				*pPattern, *pSelectedStage, Status);
			m_strStatus = std::move(Status);
			ImGui::EndDisabled();
			if (changed) return; // 변경 뒤에는 소유 도구와 cache의 포인터를 다시 조회한다.
		}
		else ImGui::EndDisabled();
	}
	const bool_t bStageSelected = !bGroupSelection && pSelectedStage &&
		m_eDetailOwner == DETAIL_OWNER::GAMEPLAY_STAGE && m_strSelectedStableId == pSelectedStage->strStageId;
	if (bStageSelected)
	{
		ImGui::BeginDisabled(!bPatternMutationAdmitted || !pPattern->bManualServerAudition || bSaveJobBlocking);
		if (ImGui::Button("Duplicate Stage"))
			(void)Execute_CompositionEdit(COMPOSITION_EDIT_COMMAND::DUPLICATE_SELECTION, m_strStatus);
		ImGui::SameLine();
		const char* label = pPattern->Stages.size() == 1u ? "Clear Stage" : "Delete Stage";
		if (ImGui::Button(label) || (keyboard && bPatternMutationAdmitted && pPattern->bManualServerAudition &&
			!bSaveJobBlocking && ImGui::IsKeyPressed(ImGuiKey_Delete, false)))
		{
			const bool changed = Delete_SelectedStage(*pPattern, *pSelectedStage, m_strStatus);
			ImGui::EndDisabled();
			if (changed) return;
		}
		else ImGui::EndDisabled();
		if (!pPattern->bManualServerAudition)
			ImGui::TextDisabled("Stage topology is protected for this encounter Pattern. Manual audition Patterns support Stage Delete.");
		else
			ImGui::TextDisabled("Deletes this Stage and its owned clips, effects, sounds and collider. The final Stage is cleared for reuse.");
	}
	Render_SelectedStageGapControl(
		*pPattern, bPatternMutationAdmitted);
	const uint32_t iTransportDurationMs = Preview.bSourceSequencePlaying ?
		(std::max)(m_iTimelineDurationMs, Preview.iSourceSequenceDurationMs) : m_iTimelineDurationMs;
	const uint32_t iRenderDurationMs = (std::min)(
		iTransportDurationMs, MAX_TIMELINE_RENDER_DURATION_MS);
	if (m_iTimelineDurationMs > iRenderDurationMs)
	{
		ImGui::TextColored(
			ImVec4(1.f, 0.72f, 0.25f, 1.f),
			"Timeline display capped at %u ms; authored duration is %u ms.",
			iRenderDurationMs, m_iTimelineDurationMs);
	}
	const float fClockCanvasWidth = (std::max)(
		1.f,
		static_cast<float>((std::max)(iRenderDurationMs, 1u)) *
			m_fTimelinePixelsPerSecond * 0.001f);
	/* 공용 timeline과 같은 24px 끝 여백을 둔다. 실제 점·label box에 공간이 더 필요할 때만 늘린다.
	   label 여백 때문에 Fit 결과가 기존 시간 범위보다 작아지면 안 된다. */
	float fCanvasWidth = fClockCanvasWidth + TIMELINE_CANVAS_END_PADDING_PX;
	for (const TIMELINE_ITEM& Item : m_TimelineItems)
	{
		if (Item.iStartMs > iRenderDurationMs)
			continue;
		const bool_t bColliderTimelineItem =
			DETAIL_OWNER::GAMEPLAY_STAGE == Item.eOwner &&
			TIMELINE_LANE::COLLIDER == Item.eLane;
		const float fDisplayWidthPx = bColliderTimelineItem ?
			TIMELINE_POINT_MINIMUM_WIDTH_PX :
			ResolveTimelineDisplayWidthPx(Item.strLabel, Item.fMinimumDisplayWidthPx);
		const float fDisplayEndX = static_cast<float>(Item.iStartMs) *
			m_fTimelinePixelsPerSecond * 0.001f + fDisplayWidthPx;
		fCanvasWidth = (std::max)(fCanvasWidth,
			fDisplayEndX + TIMELINE_CANVAS_END_PADDING_PX);
	}
	const float fTimelineHeight = metrics.rulerHeight + TIMELINE_ROW_HEIGHT *
		static_cast<float>(std::accumulate(
			m_TimelineLaneSubrowCounts.begin(),
			m_TimelineLaneSubrowCounts.end(), std::size_t{ 0u }));
	// BeginChild 전에 전체 Sequence 폭을 계산해 좌우 이동용 track이 보이게 유지한다.
	ImGui::SetNextWindowContentSize(
		ImVec2(TIMELINE_LANE_LABEL_WIDTH + fCanvasWidth, 0.f));
	if (!ImGui::BeginChild(
			"##ActionCompositionTimelineCanvas", ImVec2(0.f, 0.f),
			ImGuiChildFlags_Borders,
			ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar))
	{
		ImGui::EndChild();
		return;
	}

	const ImVec2 CanvasOrigin = ImGui::GetCursorScreenPos();
	ImGui::SetCursorScreenPos(ImVec2(CanvasOrigin.x + TIMELINE_LANE_LABEL_WIDTH, CanvasOrigin.y));
	ImGui::InvisibleButton(
		"##TimelineRuler", ImVec2(fCanvasWidth, metrics.rulerHeight));
	const ImVec2 RulerMin = ImGui::GetItemRectMin();
	const ImVec2 RulerMax = ImGui::GetItemRectMax();
	ImDrawList* const pDrawList = ImGui::GetWindowDrawList();
	CompositionTimeline::DrawRuler(
		pDrawList, RulerMin, RulerMax, iRenderDurationMs, m_fTimelinePixelsPerSecond);
	if (ImGui::IsItemActive() &&
		ImGui::IsMouseDown(ImGuiMouseButton_Left))
	{
		const float fLocalX = (std::clamp)(
			ImGui::GetIO().MousePos.x - RulerMin.x, 0.f, fCanvasWidth);
		const uint32_t iRequestedPlayheadMs = static_cast<uint32_t>((std::min)(
			static_cast<double>(iRenderDurationMs),
			static_cast<double>(fLocalX) * 1000.0 /
				static_cast<double>(m_fTimelinePixelsPerSecond)));
		if (iRequestedPlayheadMs != m_iPlayheadMs || ImGui::IsItemActivated())
		{
			m_iPlayheadMs = iRequestedPlayheadMs;
			if (nullptr != m_pAnimationTool && bLocalPreviewAdmitted)
			{
				std::string Status;
				if (sourcePreview)
					(void)m_pAnimationTool->Seek_ValtanCompositionSourceSequence(m_iPlayheadMs, true, Status);
				else
					(void)Seek_EffectivePreview(*pPattern, m_iPlayheadMs, true, Status);
				m_strStatus = std::move(Status);
			}
		}
	}

	/* 소유 도구 편집은 cache를 무효화하고 다음 프레임에 새로 결합한 Pattern으로 다시 만든다.
	   따라서 이 순회에서는 cache vector가 유지되며 모든 label/ID를 프레임마다 깊은 복사할 필요가 없다. */
	const std::vector<TIMELINE_ITEM>& TimelineItems = m_TimelineItems;
	struct MARQUEE_BOX { std::size_t index; ImVec2 min, max; };
	std::vector<MARQUEE_BOX> MarqueeBoxes;
	MarqueeBoxes.reserve(TimelineItems.size());
	if (m_bTimelineMarqueeActive && m_strTimelineMarqueePatternId != pPattern->strPatternId)
	{
		m_bTimelineMarqueeActive = false;
		m_TimelineMarqueeBefore.clear();
		m_TimelineMarqueeBase.clear();
	}
	bool_t bTimelineSoundDraftDirty = false;
	std::string TimelineSoundStatus;
	const VALTAN_PATTERN_SOUND_CUE_DOCUMENT* const pTimelineSounds =
		VALTAN_VIEW_ADMISSION::ADMITTED != m_eAdmission ||
		nullptr == m_pAnimationTool ? nullptr :
		m_pAnimationTool->Get_ValtanCompositionPatternSoundDraft(
			bTimelineSoundDraftDirty, TimelineSoundStatus);
	const bool_t bSoundMoveAdmitted =
		bMutationAdmitted && nullptr != pTimelineSounds;
	static constexpr std::array<TIMELINE_LANE, 11> TIMELINE_LANE_ORDER = {
		TIMELINE_LANE::STAGE,
		TIMELINE_LANE::ANIMATION,
		TIMELINE_LANE::LOGIC,
		TIMELINE_LANE::SUMMON,
		TIMELINE_LANE::WORLD,
		TIMELINE_LANE::SCENE_PROFILE,
		TIMELINE_LANE::EFFECT,
		TIMELINE_LANE::SOUND,
		TIMELINE_LANE::CAMERA,
		TIMELINE_LANE::COLLIDER,
		TIMELINE_LANE::LIGHT,
	};
	static_assert(TIMELINE_LANE_ORDER.size() == static_cast<std::size_t>(TIMELINE_LANE::COUNT));
	const VALTAN_STAGE_VIEW* pLaneStage = Find_SelectedStage(pPattern);
	if (nullptr == pLaneStage && !pPattern->Stages.empty())
		pLaneStage = &pPattern->Stages.front();
	CBalanceTool::PATTERN_STAGE_EDIT LaneColliderDraft;
	std::string LaneColliderDraftStatus;
	const bool_t bLaneColliderDraftReady = nullptr != pLaneStage &&
		nullptr != m_pBalanceTool &&
		m_pBalanceTool->Get_ValtanStageDraft(
			pPattern->strPatternId, pLaneStage->strStageId,
			LaneColliderDraft, LaneColliderDraftStatus);
	const bool_t bLaneColliderExists = bLaneColliderDraftReady ?
		"NONE" != LaneColliderDraft.hitShape :
		(nullptr != pLaneStage && pLaneStage->Has_HitShape());
	float fLaneY = CanvasOrigin.y + metrics.rulerHeight;
	for (const TIMELINE_LANE eLane : TIMELINE_LANE_ORDER)
	{
		const std::size_t iSubrowCount =
			m_TimelineLaneSubrowCounts[static_cast<std::size_t>(eLane)];
		const float fLaneHeight = TIMELINE_ROW_HEIGHT *
			static_cast<float>(iSubrowCount);
		ImU32 labelColor = CompositionTimeline::PresentationLabelColor;
		switch (eLane)
		{
		case TIMELINE_LANE::STAGE: labelColor = CompositionTimeline::StageLabelColor; break;
		case TIMELINE_LANE::ANIMATION: labelColor = CompositionTimeline::AnimationLabelColor; break;
		case TIMELINE_LANE::LOGIC: labelColor = CompositionTimeline::LogicLabelColor; break;
		case TIMELINE_LANE::SUMMON: labelColor = CompositionTimeline::SummonLabelColor; break;
		case TIMELINE_LANE::WORLD: labelColor = CompositionTimeline::WorldLabelColor; break;
		case TIMELINE_LANE::SCENE_PROFILE: labelColor = CompositionTimeline::SceneProfileLabelColor; break;
		default: break;
		}
		const auto LaneGeometry = CompositionTimeline::DrawCategoryLane(pDrawList,
			ImVec2(CanvasOrigin.x, fLaneY), TIMELINE_LANE_LABEL_WIDTH + fCanvasWidth,
			TIMELINE_LANE_LABEL_WIDTH, iSubrowCount, TIMELINE_ROW_HEIGHT,
			Lane_Label(eLane), labelColor, static_cast<std::size_t>(eLane) % 2u != 0u, 28.f);
		ImGui::SetCursorScreenPos(ImVec2(CanvasOrigin.x + TIMELINE_LANE_LABEL_WIDTH - 28.f, fLaneY));
		ImGui::PushID(static_cast<int32_t>(eLane));
		const bool_t bColliderLaneButtonAdmitted =
			TIMELINE_LANE::COLLIDER != eLane || bLaneColliderExists ||
			(bPatternMutationAdmitted && bLaneColliderDraftReady &&
			 LaneColliderDraft.colliderAddAdmitted);
		ImGui::BeginDisabled(
			nullptr == pLaneStage || !bColliderLaneButtonAdmitted);
		if (ImGui::SmallButton("+##LaneAuthoring") && nullptr != pLaneStage)
			Request_LaneAuthoring(eLane, *pPattern, *pLaneStage);
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		{
			if (TIMELINE_LANE::COLLIDER != eLane)
			{
				ImGui::SetTooltip(
					"Open %s authoring/detail for the selected Stage.",
					Lane_Label(eLane));
			}
			else if (bLaneColliderExists)
			{
				ImGui::SetTooltip(
					"Open the existing Server collider geometry and timing Details.");
			}
			else if (bColliderLaneButtonAdmitted)
			{
				ImGui::SetTooltip(
					"Create a Server collider box on the selected Stage.");
			}
			else if (nullptr != pLaneStage &&
				"WAIT" == pLaneStage->strSequenceRole)
			{
				ImGui::SetTooltip(
					"WAIT is clock-only; a Server collider cannot be added here.");
			}
			else if (bLaneColliderDraftReady &&
				LaneColliderDraft.colliderAddAdmitted &&
				!bPatternMutationAdmitted)
			{
				ImGui::SetTooltip(
					"Collider Add requires the admitted mutable Pattern view. Reload Canonical before editing this manual audition Stage.");
			}
			else if (bLaneColliderDraftReady)
			{
				ImGui::SetTooltip(
					"%s is %s. New collider Add is unavailable; create or select a non-WAIT manual audition Stage.",
					pPattern->strPatternId.c_str(),
					pPattern->strAdmissionState.c_str());
			}
			else if (!LaneColliderDraftStatus.empty())
			{
				ImGui::SetTooltip(
					"Collider Add unavailable: %s",
					LaneColliderDraftStatus.c_str());
			}
			else
			{
				ImGui::SetTooltip(
					"Canonical Stages without a hit are add-locked. Create or select a non-WAIT manual audition Stage.");
			}
		}
		ImGui::PopID();
		ImGui::SetCursorScreenPos(LaneGeometry.contentMin);
		ImGui::PushID(static_cast<int32_t>(eLane));
		ImGui::InvisibleButton(
			"##TimelineLaneRow", ImVec2(fCanvasWidth, fLaneHeight));
		const bool_t bLaneRowHovered = ImGui::IsItemHovered();
		bool_t bLaneBlockHovered = false;
		const ImVec2 RowMin = ImGui::GetItemRectMin();
		const ImVec2 RowMax = ImGui::GetItemRectMax();

		for (std::size_t iItem = 0u; iItem < TimelineItems.size(); ++iItem)
		{
			const TIMELINE_ITEM& Item = TimelineItems[iItem];
			if (Item.eLane != eLane || Item.iStartMs > iRenderDurationMs)
				continue;
			const uint32_t iItemEndMs = (std::min)(
				Item.iEndMs, iRenderDurationMs);
			ImGui::PushID(static_cast<int32_t>(iItem));
		const float fStartX = RowMin.x +
			static_cast<float>(Item.iStartMs) *
			m_fTimelinePixelsPerSecond * 0.001f;
		const bool_t bColliderTimelineItem =
			DETAIL_OWNER::GAMEPLAY_STAGE == Item.eOwner &&
			TIMELINE_LANE::COLLIDER == Item.eLane;
		/* Collider 폭은 label 배치가 아니라 판정 schedule을 표현한다.
		   여기서 label의 최소 폭을 적용하면 250→900ms pulse 구간보다 box가 넓어져 경계 편집이 보이지 않을 수 있다. */
		const float fDisplayWidthPx = bColliderTimelineItem ?
			TIMELINE_POINT_MINIMUM_WIDTH_PX :
			ResolveTimelineDisplayWidthPx(
				Item.strLabel, Item.fMinimumDisplayWidthPx);
		const float fSemanticEndX = RowMin.x +
			static_cast<float>(iItemEndMs) *
			m_fTimelinePixelsPerSecond * 0.001f;
		const float fEndX = (std::max)(
			fStartX + fDisplayWidthPx,
			fSemanticEndX);
		const float fSubrowY = LaneGeometry.SubrowOrigin(Item.iSubrow).y;
		const ImVec2 BlockMin(fStartX, fSubrowY);
		const ImVec2 BlockMax(
			(std::min)(fEndX, RowMax.x), fSubrowY + TIMELINE_BLOCK_HEIGHT);
		const ImVec2 HitMin(fStartX, fSubrowY);
		const ImVec2 HitMax(
			(std::min)(fEndX, RowMax.x),
			fSubrowY + TIMELINE_ROW_HEIGHT);
		MarqueeBoxes.push_back({iItem, BlockMin, BlockMax});
		const bool_t bSelectedBlock = !m_TimelineSelection.empty() ? Is_TimelineBoxSelected(Item) :
			Item.strPatternId == m_strSelectedPatternId &&
			Item.strStageId == m_strSelectedStageId &&
			((Item.eOwner == m_eDetailOwner && Item.strStableId == m_strSelectedStableId) ||
			 (Item.eLane == TIMELINE_LANE::ANIMATION && m_eDetailOwner == DETAIL_OWNER::GAMEPLAY_STAGE &&
			  m_strSelectedStableId == m_strSelectedStageId));
		const bool_t bBlockHovered = bLaneRowHovered &&
			ImGui::IsMouseHoveringRect(HitMin, HitMax, true);
		bLaneBlockHovered |= bBlockHovered;
		if (bBlockHovered)
		{
			ImGui::SetTooltip(
				"%s\n%u - %u ms", Item.strLabel.c_str(),
				Item.iStartMs, Item.iEndMs);
		}
		const auto ItemStage = std::find_if(
			pPattern->Stages.begin(), pPattern->Stages.end(),
			[&Item](const VALTAN_STAGE_VIEW& Stage)
			{
				return Stage.strStageId == Item.strStageId;
			});
		const VALTAN_PRODUCT_EFFECT_CUE_VIEW* pEffectCue = nullptr;
		if (DETAIL_OWNER::EFFECT == Item.eOwner &&
			ItemStage != pPattern->Stages.end())
		{
			const auto Cue = std::find_if(
				ItemStage->ProductCues.begin(), ItemStage->ProductCues.end(),
				[&Item](const VALTAN_PRODUCT_EFFECT_CUE_VIEW& Candidate)
				{
					return Candidate.strOccurrenceId == Item.strStableId;
				});
			if (ItemStage->ProductCues.end() != Cue)
				pEffectCue = &*Cue;
		}
		const VALTAN_PATTERN_SOUND_CUE* pSoundCue = nullptr;
		if (DETAIL_OWNER::SOUND == Item.eOwner &&
			nullptr != pTimelineSounds && ItemStage != pPattern->Stages.end())
		{
			const auto Cue = std::find_if(
				pTimelineSounds->Cues.begin(), pTimelineSounds->Cues.end(),
				[&Item, &pPattern, &ItemStage](
					const VALTAN_PATTERN_SOUND_CUE& Candidate)
				{
					return Candidate.strOccurrenceId == Item.strStableId &&
						Candidate.strPatternId == pPattern->strPatternId &&
						Candidate.strStageId == ItemStage->strStageId &&
						Candidate.strActionId == ItemStage->strActionId;
				});
			if (pTimelineSounds->Cues.end() != Cue)
				pSoundCue = &*Cue;
		}
		const bool_t bEffectRightTrim = nullptr != pEffectCue &&
			("natural" == pEffectCue->strStopPolicy ||
			 (pEffectCue->bHasSourceEnd && "cue_end" == pEffectCue->strStopPolicy)) &&
			"once" == pEffectCue->strRepeatPolicy;
		const bool_t bEffectStartTrim = bEffectRightTrim;
		const bool_t bSoundTrim = nullptr != pSoundCue && Item.iEndMs > Item.iStartMs &&
			pSoundCue->eRepeatPolicy == VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
		const bool_t bCounterLogic =
			DETAIL_OWNER::GAMEPLAY_STAGE == Item.eOwner &&
			TIMELINE_LANE::LOGIC == Item.eLane &&
			Item.strStableId == BuildBranchStableId(
				Item.strStageId, "COUNTER_HIT");
		const bool_t bColliderSchedule =
			DETAIL_OWNER::GAMEPLAY_STAGE == Item.eOwner &&
			TIMELINE_LANE::COLLIDER == Item.eLane && Item.bEditable &&
			ItemStage != pPattern->Stages.end();
		/* pulse 하나는 길이가 없는 한 시점이다.
		   활성 수명이나 서로 다른 두 pulse가 있어야 양쪽 경계를 독립적으로 조절할 수 있다. */
		const bool_t bColliderCanResize = bColliderSchedule &&
			Item.iEndMs > Item.iStartMs;
		const bool_t bAuxiliaryTimed = Item.bEditable &&
			(DETAIL_OWNER::SCENE_PROFILE == Item.eOwner || DETAIL_OWNER::LIGHT == Item.eOwner ||
			 (DETAIL_OWNER::CAMERA == Item.eOwner && ItemStage != pPattern->Stages.end() &&
			  std::any_of(ItemStage->CameraInvocations.begin(), ItemStage->CameraInvocations.end(), [&](const auto& x) { return x.strCameraInvocationId == Item.strStableId; })) ||
			 (DETAIL_OWNER::COMBAT_OBJECT == Item.eOwner && TIMELINE_LANE::SUMMON == Item.eLane));
		const bool_t bAuxiliaryMove = Item.bEditable && (bAuxiliaryTimed ||
			(DETAIL_OWNER::GAMEPLAY_STAGE == Item.eOwner &&
			 ((TIMELINE_LANE::STAGE == Item.eLane && Item.iStartMs != 0u) ||
			  Item.strStableId.find("/action/") != std::string::npos)));
        const bool_t bAnimationSourceTrim = DETAIL_OWNER::ANIMATION == Item.eOwner &&
            Item.bEditable && !Item.bLoopsToStageEnd && ItemStage != pPattern->Stages.end();
        const bool_t bStartTrimOwner = bColliderCanResize || bAuxiliaryTimed || bAnimationSourceTrim || bEffectStartTrim || bSoundTrim;
		const bool_t bTrimOwner =
			(DETAIL_OWNER::GAMEPLAY_STAGE == Item.eOwner &&
			 (TIMELINE_LANE::STAGE == Item.eLane || bCounterLogic ||
			  bColliderCanResize)) ||
			DETAIL_OWNER::ANIMATION == Item.eOwner || bEffectRightTrim || bAuxiliaryTimed || bSoundTrim;
		const bool_t bTrimMutationAdmitted = bPatternMutationAdmitted &&
			nullptr != m_pBalanceTool;
		const std::string& boxLabel = Item.eLane == TIMELINE_LANE::STAGE ? Item.strStageId :
			(Item.eLane == TIMELINE_LANE::ANIMATION && !Item.strAssetId.empty() ? Item.strAssetId : Item.strLabel);
		CompositionTimeline::DrawBox(pDrawList, BlockMin, BlockMax,
			Lane_Color(Item.eLane), bSelectedBlock, boxLabel.c_str(),
			bStartTrimOwner && bTrimMutationAdmitted,
			Item.bEditable && bTrimOwner && bTrimMutationAdmitted);
		const float fEndTrimHandleX = bColliderSchedule ?
			(std::min)(fSemanticEndX, RowMax.x) : BlockMax.x;
		const bool_t bTrimRowHovered = bLaneRowHovered &&
			ImGui::GetIO().MousePos.y >= BlockMin.y &&
			ImGui::GetIO().MousePos.y <= BlockMax.y;
		const CompositionTimeline::BoxGesture gesture = CompositionTimeline::HitBoxGesture(
			ImGui::GetIO().MousePos.x, BlockMin.x, fEndTrimHandleX, 7.f,
			bStartTrimOwner, Item.bEditable && bTrimOwner);
		const bool_t bStartTrimHandleHovered = bTrimRowHovered &&
			CompositionTimeline::BoxGesture::TRIM_START == gesture;
		const bool_t bEndTrimHandleHovered = bTrimRowHovered &&
			CompositionTimeline::BoxGesture::TRIM_END == gesture;
		const bool_t bTrimHandleHovered =
			bStartTrimHandleHovered || bEndTrimHandleHovered;
		const bool_t bThisTrimActive = m_bTimelineTrimActive &&
			m_strTimelineTrimPatternId == Item.strPatternId &&
			m_strTimelineTrimStageId == Item.strStageId &&
			m_strTimelineTrimStableId == Item.strStableId;
		const bool_t bStageMove = TIMELINE_LANE::STAGE == Item.eLane &&
			DETAIL_OWNER::GAMEPLAY_STAGE == Item.eOwner && Item.bEditable &&
			pPattern->bManualServerAudition && ItemStage != pPattern->Stages.end();
		const bool_t bAnimationMove =
			DETAIL_OWNER::ANIMATION == Item.eOwner &&
			TIMELINE_LANE::ANIMATION == Item.eLane && Item.bEditable &&
			!Item.bLoopsToStageEnd &&
			ItemStage != pPattern->Stages.end();
		const bool_t bMoveOwner = bStageMove || bAuxiliaryMove || bAnimationMove || bColliderSchedule ||
			Item.bEffectV2Binding ||
			nullptr != pEffectCue ||
			nullptr != pSoundCue;
		const bool_t bMoveMutationAdmitted =
			(bStageMove && bPatternMutationAdmitted && nullptr != m_pBalanceTool) ||
			(bAuxiliaryMove && bPatternMutationAdmitted && nullptr != m_pBalanceTool) ||
			(bAnimationMove && bPatternMutationAdmitted) ||
			(bColliderSchedule && bPatternMutationAdmitted &&
			 nullptr != m_pBalanceTool) ||
			(Item.bEffectV2Binding && bMutationAdmitted) ||
			(nullptr != pEffectCue && bPatternMutationAdmitted) ||
			(nullptr != pSoundCue && bSoundMoveAdmitted);
		const bool_t bThisMoveActive = m_bTimelineMoveActive &&
			m_strTimelineMovePatternId == Item.strPatternId &&
			m_strTimelineMoveStageId == Item.strStageId &&
			m_strTimelineMoveStableId == Item.strStableId;
		const bool_t bGroupMovePreview = m_bTimelineMoveActive && m_TimelineSelection.size() > 1u && Is_TimelineBoxSelected(Item);
		if (bThisMoveActive || bThisTrimActive || bGroupMovePreview)
		{
			const int64_t mouseMs = static_cast<int64_t>(std::llround((std::max)(0.f, ImGui::GetIO().MousePos.x - RowMin.x) * 1000.f / m_fTimelinePixelsPerSecond));
			const int64_t delta = mouseMs - static_cast<int64_t>((bThisMoveActive || bGroupMovePreview) ? m_iTimelineMoveMouseStartMs : m_iTimelineTrimMouseStartMs);
			int64_t start = Item.iStartMs, end = Item.iEndMs;
			if (bThisMoveActive || bGroupMovePreview) { start += delta; end += delta; }
			else if (m_bTimelineTrimStartEdge && bAnimationSourceTrim)
            { end = (std::max)(start + 1, end - delta); }
            else if (m_bTimelineTrimStartEdge) start = (std::min)(start + delta, end - 1);
			else end = (std::max)(end + delta, start + 1);
			const float x1 = RowMin.x + static_cast<float>((std::max)(int64_t{0}, start)) * m_fTimelinePixelsPerSecond * .001f;
			const float x2 = (std::max)(x1 + 4.f, RowMin.x + static_cast<float>((std::max)(int64_t{0}, end)) * m_fTimelinePixelsPerSecond * .001f);
			pDrawList->AddRectFilled(ImVec2(x1, BlockMin.y), ImVec2(x2, BlockMax.y), IM_COL32(255, 255, 255, 55), 3.f);
			pDrawList->AddRect(ImVec2(x1, BlockMin.y), ImVec2(x2, BlockMax.y), IM_COL32(255, 220, 72, 255), 3.f, 0, 2.f);
		}
		if (bTrimHandleHovered || bThisTrimActive)
		{
			const float fTrimEdgeX = bThisTrimActive &&
				m_bTimelineTrimStartEdge ? BlockMin.x :
				(bStartTrimHandleHovered ? BlockMin.x : fEndTrimHandleX);
			pDrawList->AddLine(
				ImVec2(fTrimEdgeX, BlockMin.y),
				ImVec2(fTrimEdgeX, BlockMax.y),
				IM_COL32(255, 255, 255, 255), 3.f);
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
		}
		if (bBlockHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			const bool toggle = ImGui::GetIO().KeyShift;
			const bool keepGroupForDrag = !toggle && !bTrimHandleHovered && bMoveOwner &&
				bMoveMutationAdmitted && m_TimelineSelection.size() > 1u && Is_TimelineBoxSelected(Item);
			if (toggle && m_TimelineSelection.empty())
			{
				const auto previous = std::find_if(TimelineItems.begin(), TimelineItems.end(), [&](const auto& row) {
					return row.strPatternId == m_strSelectedPatternId &&
						row.strStageId == m_strSelectedStageId && row.strStableId == m_strSelectedStableId && row.eOwner == m_eDetailOwner;
				});
				if (previous != TimelineItems.end()) m_TimelineSelection.push_back(Timeline_SelectionKey(*previous));
			}
			m_bDetailsWindowVisible = true;
			if (const VALTAN_STAGE_VIEW* const pStage = Find_SelectedStage(pPattern);
				nullptr == pStage || pStage->strStageId != Item.strStageId)
			{
				const auto FoundStage = std::find_if(
					pPattern->Stages.begin(), pPattern->Stages.end(),
					[&Item](const VALTAN_STAGE_VIEW& Stage)
					{
						return Stage.strStageId == Item.strStageId;
					});
				if (FoundStage != pPattern->Stages.end())
					Select_Stage(*pPattern, *FoundStage, Item.eOwner, Item.strStableId, true);
			}
			else
			{
				Select_Stage(*pPattern, *pStage, Item.eOwner, Item.strStableId, true);
			}
			if (!keepGroupForDrag) Select_TimelineBox(Item, toggle);
            if (!toggle && ((bTrimHandleHovered && bTrimMutationAdmitted) || (bMoveOwner && bMoveMutationAdmitted)))
            {
                m_iTimelineGestureDraftGeneration = m_pBalanceTool ? m_pBalanceTool->Get_ValtanDraftGeneration() : 0u;
                m_iTimelineGestureSoundGeneration = m_pAnimationTool ? m_pAnimationTool->Get_ValtanCompositionPatternSoundDraftGeneration() : 0u;
                m_iTimelineGestureEffectRevision = CEffectV2Catalog::Get().Get_Revision();
            }
			if (!toggle && bTrimHandleHovered && bTrimMutationAdmitted)
			{
				bool_t bBeginTrim = true;
				uint32_t iSourceEndpointMs = 0u;
				std::string TrimStatus;
				if (bColliderSchedule)
				{
					CBalanceTool::PATTERN_STAGE_EDIT ColliderDraft;
					uint32_t iFirstMs = 0u;
					uint32_t iLastMs = 0u;
					bBeginTrim = m_pBalanceTool->Get_ValtanStageDraft(
						Item.strPatternId, Item.strStageId,
						ColliderDraft, TrimStatus) &&
						ResolveValtanColliderScheduleEndpoints(
							ColliderDraft, iFirstMs, iLastMs, TrimStatus);
					iSourceEndpointMs = bStartTrimHandleHovered ?
						iFirstMs : iLastMs;
				}
				if (bBeginTrim)
				{
					m_strSelectedStableId = Item.strStableId;
					m_eDetailOwner = Item.eOwner;
					m_bTimelineTrimActive = true;
					m_bTimelineTrimStartEdge = bStartTrimHandleHovered;
					m_strTimelineTrimPatternId = Item.strPatternId;
					m_strTimelineTrimStageId = Item.strStageId;
					m_strTimelineTrimStableId = Item.strStableId;
					const float fMouseTimelineMs = (std::clamp)(
						(ImGui::GetIO().MousePos.x - RowMin.x) * 1000.f /
							m_fTimelinePixelsPerSecond,
						0.f, static_cast<float>(iRenderDurationMs));
					m_iTimelineTrimMouseStartMs = static_cast<uint32_t>(
						std::llround(fMouseTimelineMs));
					m_iTimelineTrimSourceEndpointMs = iSourceEndpointMs;
				}
				else
				{
					m_strStatus = std::move(TrimStatus);
				}
			}
			else if (!toggle && bMoveOwner && bMoveMutationAdmitted)
			{
				m_bTimelineMoveActive = true;
				m_strTimelineMovePatternId = Item.strPatternId;
				m_strTimelineMoveStageId = Item.strStageId;
				m_strTimelineMoveStableId = Item.strStableId;
				const float fMouseTimelineMs = (std::clamp)(
					(ImGui::GetIO().MousePos.x - RowMin.x) * 1000.f /
						m_fTimelinePixelsPerSecond,
					0.f, static_cast<float>(iRenderDurationMs));
				m_iTimelineMoveMouseStartMs = static_cast<uint32_t>(
					std::llround(fMouseTimelineMs));
				if (Item.bEffectV2Binding)
				{
					m_iTimelineMoveSourceStartMs =
						Item.iEffectV2BindingStartMs;
					m_iTimelineMoveSourceEndMs = 0u;
				}
				else
				{
					m_iTimelineMoveSourceStartMs = nullptr != pEffectCue ?
						pEffectCue->iSourceStartMs :
						(nullptr != pSoundCue ? pSoundCue->iStartMs : 0u);
					m_iTimelineMoveSourceEndMs = nullptr != pEffectCue &&
						pEffectCue->bHasSourceEnd ? pEffectCue->iSourceEndMs : 0u;
				}
			}
		}
		if (bThisTrimActive && bTrimMutationAdmitted &&
			ImGui::IsMouseReleased(ImGuiMouseButton_Left) &&
			nullptr != m_pBalanceTool)
		{
			const float fMouseTimelineMs = (std::max)(
				0.f, ImGui::GetIO().MousePos.x - RowMin.x) * 1000.f /
				m_fTimelinePixelsPerSecond;
            const int64_t trimDelta = static_cast<int64_t>(std::llround(fMouseTimelineMs)) - m_iTimelineTrimMouseStartMs;
            const uint32_t iNewWallMs = static_cast<uint32_t>((std::clamp)(
                static_cast<int64_t>(Item.iEndMs - Item.iStartMs) + trimDelta,
                int64_t{1}, int64_t{120000}));
			std::string Status;
			bool_t bApplied = false;
			if (bSoundTrim)
			{
				const uint32_t offset = pSoundCue->bHasPlaybackOffset ? pSoundCue->iPlaybackOffsetMs : 0u;
				const int64_t first = m_bTimelineTrimStartEdge ? (std::clamp)(
					int64_t(Item.iStartMs) + trimDelta, (std::max)(int64_t{0}, int64_t(Item.iStartMs) - offset),
					int64_t(Item.iEndMs) - 1) : Item.iStartMs;
				const int64_t end = m_bTimelineTrimStartEdge ? Item.iEndMs :
					(std::clamp)(int64_t(Item.iEndMs) + trimDelta, first + 1, int64_t{600000});
				const int64_t nextOffset = int64_t(offset) + first - Item.iStartMs;
				if (nextOffset >= 0 && nextOffset <= 600000)
					bApplied = Apply_SoundTimelinePlacement(*pPattern, *ItemStage, *pSoundCue,
						static_cast<uint32_t>(first), static_cast<uint32_t>(nextOffset),
						static_cast<uint32_t>(end - first), Status);
			}
			else if (bAuxiliaryTimed)
			{
				const int64_t delta = static_cast<int64_t>(std::llround(fMouseTimelineMs)) - m_iTimelineTrimMouseStartMs;
				const uint32_t begin = m_bTimelineTrimStartEdge ? static_cast<uint32_t>((std::clamp)(static_cast<int64_t>(Item.iStartMs) + delta, int64_t{0}, static_cast<int64_t>(Item.iEndMs) - 1)) : Item.iStartMs;
				const uint32_t end = m_bTimelineTrimStartEdge ? Item.iEndMs : static_cast<uint32_t>((std::clamp)(static_cast<int64_t>(Item.iEndMs) + delta, static_cast<int64_t>(begin) + 1, int64_t{600000}));
				bApplied = Apply_AuxiliaryTimelineTiming(*pPattern, Item, begin, end, true, Status);
			}
			else if (bColliderSchedule)
			{
				CBalanceTool::PATTERN_STAGE_EDIT Draft;
				const uint32_t iMouseTimelineMs = static_cast<uint32_t>(
					std::llround((std::clamp)(
						fMouseTimelineMs, 0.f,
						static_cast<float>(iRenderDurationMs))));
				/* 마우스 시각은 Pattern 전체 기준이고 잡은 끝점은 Stage 내부 기준이다.
				   마우스의 이동 차이는 원점과 무관하므로 그 차이만 local 끝점에 더한다.
				   불변 frame 투영에서 얻은 오래된 Stage 시작 시각을 다시 빼지 않는다. */
				const int64_t iRequestedEndpointMs =
					static_cast<int64_t>(m_iTimelineTrimSourceEndpointMs) +
					static_cast<int64_t>(iMouseTimelineMs) -
					static_cast<int64_t>(m_iTimelineTrimMouseStartMs);
				bApplied = m_pBalanceTool->Get_ValtanStageDraft(
						Item.strPatternId, Item.strStageId, Draft, Status) &&
					ResizeValtanColliderScheduleEndpoint(
						Draft, m_bTimelineTrimStartEdge,
						static_cast<uint32_t>((std::clamp)(
							iRequestedEndpointMs, int64_t{ 0 },
							static_cast<int64_t>(Draft.hasHitActivation ?
								Draft.durationMs : Draft.durationMs - 1u))),
						Status) &&
					SetValtanStageDraftWithSoundDependencyAdmission(
						m_pAnimationTool, m_pBalanceTool,
						m_bPatternShakesReady ? &m_PatternShakes : nullptr,
						*pPattern, *ItemStage, Draft, Status);
			}
			else if (ItemStage != pPattern->Stages.end() &&
				DETAIL_OWNER::GAMEPLAY_STAGE == Item.eOwner &&
				(TIMELINE_LANE::STAGE == Item.eLane || bCounterLogic))
			{
				CBalanceTool::PATTERN_STAGE_EDIT Draft;
				bApplied = m_pBalanceTool->Get_ValtanStageDraft(
						Item.strPatternId, Item.strStageId, Draft, Status) &&
					ApplyStageClockPolicy(Draft, iNewWallMs, Status) &&
					SetValtanStageDraftWithSoundDependencyAdmission(
						m_pAnimationTool, m_pBalanceTool,
						m_bPatternShakesReady ? &m_PatternShakes : nullptr,
						*pPattern, *ItemStage, Draft, Status);
			}
			else if (ItemStage != pPattern->Stages.end() &&
				DETAIL_OWNER::ANIMATION == Item.eOwner)
			{
				CBalanceTool::PATTERN_STAGE_EDIT Draft;
				if (m_pBalanceTool->Get_ValtanStageDraft(
						Item.strPatternId, Item.strStageId, Draft, Status))
				{
					const auto Slot = std::find_if(
						Draft.animationSlots.begin(), Draft.animationSlots.end(),
						[&Item](const CBalanceTool::ANIMATION_SLOT_EDIT& Candidate)
						{
							return Candidate.clipOccurrenceId == Item.strStableId;
						});
					if (Draft.animationSlots.end() != Slot &&
						!Slot->repeatUntilStageEnd &&
						std::isfinite(Slot->playRate) && Slot->playRate > 0.0)
					{
                        bApplied = Trim_AnimationOccurrence(*pPattern, *ItemStage,
                            Item.strStableId, m_bTimelineTrimStartEdge, trimDelta, Status);
					}
					else if (Draft.animationSlots.end() != Slot &&
						Slot->repeatUntilStageEnd)
					{
						/* 반복 slot의 끝은 Server Stage 시각이다.
						   따라서 오른쪽 경계 드래그는 Stage box와 같은 시각 정책을 통해 Stage 길이를 수정한다. */
						const TIMELINE_ITEM* pStageItem = nullptr;
						for (const TIMELINE_ITEM& Candidate : TimelineItems)
						{
							if (TIMELINE_LANE::STAGE == Candidate.eLane &&
								DETAIL_OWNER::GAMEPLAY_STAGE == Candidate.eOwner &&
								Candidate.strPatternId == Item.strPatternId &&
								Candidate.strStageId == Item.strStageId)
							{
								pStageItem = &Candidate;
								break;
							}
						}
						const uint32_t iSlotStageLocalStartMs =
							nullptr == pStageItem ||
							Item.iStartMs < pStageItem->iStartMs ?
							0u : Item.iStartMs - pStageItem->iStartMs;
						const uint64_t iRequestedStageMs =
							static_cast<uint64_t>(iSlotStageLocalStartMs) + iNewWallMs;
						bApplied = Draft.durationEditable &&
							iRequestedStageMs >= 1u && iRequestedStageMs <= 600000u &&
							ApplyStageClockPolicy(
								Draft, static_cast<uint32_t>(iRequestedStageMs), Status) &&
							SetValtanStageDraftWithSoundDependencyAdmission(
								m_pAnimationTool, m_pBalanceTool,
								m_bPatternShakesReady ? &m_PatternShakes : nullptr,
								*pPattern, *ItemStage, Draft, Status);
						if (!bApplied && Status.empty())
						{
							Status =
								"Loop slot right-edge trim requires an editable Stage clock.";
						}
					}
					else
					{
						Status =
							"Animation right-edge trim requires one finite non-loop occurrence.";
					}
				}
			}
			else if (ItemStage != pPattern->Stages.end() &&
				nullptr != pEffectCue && bEffectRightTrim)
			{
				const auto Clip = std::find_if(
					ItemStage->ClipOccurrences.begin(),
					ItemStage->ClipOccurrences.end(),
					[pEffectCue](const VALTAN_CLIP_OCCURRENCE_VIEW& Candidate)
					{
						return Candidate.strClipOccurrenceId ==
							pEffectCue->strClipOccurrenceId;
					});
				if (trimDelta == 0)
				{
					Status = "Effect timing draft is unchanged.";
				}
				else if (pEffectCue->bUsesStageClock || (ItemStage->ClipOccurrences.end() != Clip &&
					std::isfinite(Clip->fPlayRate) && Clip->fPlayRate > 0.f))
				{
                    const double rate = pEffectCue->bUsesStageClock ? 1.0 : double(Clip->fPlayRate);
                    if (m_bTimelineTrimStartEdge)
                    {
                        const uint32_t offset = ResolveEffectPlaybackOffsetMs(*pEffectCue);
                        const int64_t minimumStart = (std::max)(int64_t{0},
                            int64_t(Item.iStartMs) - static_cast<int64_t>(std::floor(offset / rate)));
                        const uint32_t start = static_cast<uint32_t>((std::clamp)(
                            int64_t(Item.iStartMs) + trimDelta, minimumStart, int64_t(Item.iEndMs) - 1));
                        const int64_t nextOffset = int64_t(offset) + std::llround(
                            (int64_t(start) - Item.iStartMs) * rate);
                        if (nextOffset < 0 || nextOffset > 600000)
                            Status = "Effect In must remain within 0..600000 ms; the cue was preserved.";
                        else if (start != Item.iStartMs)
                        {
                            auto candidate = *pEffectCue;
                            candidate.bHasPlaybackOffset = true;
                            candidate.iPlaybackOffsetMs = static_cast<uint32_t>(nextOffset);
                            std::string targetStage, targetOccurrence;
                            bApplied = MoveValtanEffectInvocation(m_pAnimationTool, m_pBalanceTool,
                                *pPattern, *ItemStage, candidate, TimelineItems, start,
                                targetStage, targetOccurrence, Status, Item.iEndMs);
                            if (bApplied)
                            {
                                m_strSelectedStageId = targetStage;
                                m_strSelectedStableId = targetOccurrence;
                                m_strEffectEditIdentity.clear();
                                Status = "Trimmed Effect In to " + std::to_string(nextOffset) +
                                    " ms; Effect Out is unchanged. Save commits this playback window.";
                            }
                        }
                    }
                    else
                    {
					const uint64_t iRequestedSourceEnd =
						static_cast<uint64_t>(pEffectCue->iSourceStartMs) +
						static_cast<uint64_t>(std::llround(
							static_cast<double>(iNewWallMs) * rate));
					const uint64_t iMaximumSourceEnd = 600000u;
					const uint32_t iCandidateSourceEnd = static_cast<uint32_t>(
						(std::clamp)(
							iRequestedSourceEnd,
							static_cast<uint64_t>(pEffectCue->iSourceStartMs) + 1u,
							iMaximumSourceEnd));
					bApplied = Apply_EffectOccurrenceTiming(
						*pPattern, *ItemStage, *pEffectCue,
						pEffectCue->iSourceStartMs, iCandidateSourceEnd,
						Status);
                    }
				}
				else
				{
					Status =
						"Effect right-edge trim lost its exact finite source-rate window.";
				}
			}
			if (bApplied)
			{
				if (bColliderSchedule || bAuxiliaryTimed || bEffectRightTrim)
					Refresh_PatternLocalPreviewAfterMutation(pPattern, Status);
				m_strStatus = std::move(Status);
				Invalidate_TimelineCache();
			}
			else
			{
				m_strStatus = std::move(Status);
			}
		}
		if (bThisMoveActive && bMoveMutationAdmitted &&
			ImGui::IsMouseReleased(ImGuiMouseButton_Left) &&
			ItemStage != pPattern->Stages.end())
		{
			const float fMouseTimelineMs = (std::clamp)(
				(ImGui::GetIO().MousePos.x - RowMin.x) * 1000.f /
					m_fTimelinePixelsPerSecond,
				0.f, static_cast<float>(iRenderDurationMs));
			const int64_t iWallDeltaMs = static_cast<int64_t>(std::llround(
				fMouseTimelineMs)) -
				static_cast<int64_t>(m_iTimelineMoveMouseStartMs);
			/* Pattern 전체 시각의 이동 차이는 Stage 내부 시각의 이동 차이와 같다.
			   타입별 helper가 전체 timing 구간을 해당 Stage 범위에 맞춘다. */
			if (m_TimelineSelection.size() > 1u)
			{
				m_bTimelineMoveActive = false;
				const ImVec2 drag = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
				if (drag.x == 0.f && drag.y == 0.f)
					Select_TimelineBox(Item, false);
				else
					(void)Apply_TimelineGroupEdit(*pPattern, TIMELINE_GROUP_EDIT::MOVE_BOXES, iWallDeltaMs, m_strStatus);
			}
			else if (bStageMove)
			{
				m_bTimelineMoveActive = false;
				(void)Move_SelectedStages(*pPattern, iWallDeltaMs, m_strStatus);
			}
			else if (bAuxiliaryMove)
			{
				const uint32_t begin = static_cast<uint32_t>((std::clamp)(static_cast<int64_t>(Item.iStartMs) + iWallDeltaMs, int64_t{0}, int64_t{600000}));
				const uint32_t end = begin + (Item.iEndMs - Item.iStartMs);
				std::string status;
				if (Apply_AuxiliaryTimelineTiming(*pPattern, Item, begin, end, false, status))
				{
					Invalidate_TimelineCache();
					Refresh_PatternLocalPreviewAfterMutation(pPattern, status);
				}
				m_strStatus = std::move(status);
			}
			else if (bColliderSchedule)
			{
				CBalanceTool::PATTERN_STAGE_EDIT Draft;
				std::string Status;
				const bool_t bApplied =
					m_pBalanceTool->Get_ValtanStageDraft(
						Item.strPatternId, Item.strStageId, Draft, Status) &&
					TranslateValtanColliderSchedule(
						Draft, iWallDeltaMs, Status) &&
					SetValtanStageDraftWithSoundDependencyAdmission(
						m_pAnimationTool, m_pBalanceTool,
						m_bPatternShakesReady ? &m_PatternShakes : nullptr,
						*pPattern, *ItemStage, Draft, Status);
				if (bApplied)
					Refresh_PatternLocalPreviewAfterMutation(pPattern, Status);
				m_strStatus = std::move(Status);
			}
			else if (bAnimationMove)
			{
				const uint32_t iDropMs = static_cast<uint32_t>(
					std::llround(fMouseTimelineMs));
				const TIMELINE_ITEM* pTargetStageItem = nullptr;
				for (const TIMELINE_ITEM& Candidate : TimelineItems)
				{
					if (TIMELINE_LANE::STAGE != Candidate.eLane ||
						DETAIL_OWNER::GAMEPLAY_STAGE != Candidate.eOwner ||
						Candidate.strPatternId != Item.strPatternId ||
						iDropMs < Candidate.iStartMs ||
						iDropMs > Candidate.iEndMs)
					{
						continue;
					}
					/* 공유 경계에 놓았을 때는 drop 시각에서 시작하는 Stage를 우선한다.
					   오른쪽으로 이동하면 다음 Stage에 자연스럽게 들어가게 한다. */
					if (nullptr == pTargetStageItem ||
						Candidate.iStartMs >= pTargetStageItem->iStartMs)
					{
						pTargetStageItem = &Candidate;
					}
				}
				const VALTAN_STAGE_VIEW* pTargetStage = nullptr;
				if (nullptr != pTargetStageItem)
				{
					const auto TargetStage = std::find_if(
						pPattern->Stages.begin(), pPattern->Stages.end(),
						[pTargetStageItem](const VALTAN_STAGE_VIEW& Stage)
						{
							return Stage.strStageId ==
								pTargetStageItem->strStageId;
						});
					if (pPattern->Stages.end() != TargetStage)
						pTargetStage = &*TargetStage;
				}
				std::size_t iTargetIndex = 0u;
				for (const TIMELINE_ITEM& Candidate : TimelineItems)
				{
					if (nullptr == pTargetStage ||
						TIMELINE_LANE::ANIMATION != Candidate.eLane ||
						DETAIL_OWNER::ANIMATION != Candidate.eOwner ||
						Candidate.strStageId != pTargetStage->strStageId ||
						Candidate.strStableId == Item.strStableId)
					{
						continue;
					}
					const uint32_t iMidpointMs = Candidate.iStartMs +
						(Candidate.iEndMs - Candidate.iStartMs) / 2u;
					if (iDropMs >= iMidpointMs)
						++iTargetIndex;
				}
				std::string Status;
				if (nullptr == pTargetStage)
				{
					Status =
						"Animation drag must end over one visible Stage clock.";
				}
				else
				{
					(void)Transfer_AnimationOccurrence(
						*pPattern, *ItemStage, *pTargetStage,
						Item.strStableId, iTargetIndex, Status);
				}
				m_strStatus = std::move(Status);
			}
			else if (Item.bEffectV2Binding)
			{
				// 마우스로 잡은 지점이 아니라 이동된 box의 시작 시각을 새 소유 Stage의 시각으로 변환한다.
				const auto ProjectV2Move = [](const VALTAN_PATTERN_VIEW& Pattern,
					const std::vector<TIMELINE_ITEM>& Items, const EFFECT_V2_BINDING& Source,
					const uint32_t StartMs, EFFECT_V2_BINDING& OutBinding,
					const VALTAN_STAGE_VIEW*& OutStage, std::string& Status) -> bool_t
				{
					const TIMELINE_ITEM* StageBox = nullptr;
					for (const auto& Box : Items)
						if (Box.eLane == TIMELINE_LANE::STAGE && Box.eOwner == DETAIL_OWNER::GAMEPLAY_STAGE &&
							Box.strPatternId == Pattern.strPatternId && StartMs >= Box.iStartMs && StartMs < Box.iEndMs &&
							(!StageBox || Box.iStartMs > StageBox->iStartMs)) StageBox = &Box;
					if (!StageBox || Source.strPatternId != Pattern.strPatternId)
					{ Status = "V2 drag must begin inside a Stage; the existing binding was preserved."; return false; }
					const auto Stage = std::find_if(Pattern.Stages.begin(), Pattern.Stages.end(),
						[&](const auto& Row) { return Row.strStageId == StageBox->strStageId; });
					if (Stage == Pattern.Stages.end() || Stage->strSequenceRole == "WAIT")
					{ Status = "V2 drag requires an editable non-WAIT Stage; the binding was preserved."; return false; }
					auto Candidate = Source;
					Candidate.strStageId = Stage->strStageId;
					Candidate.strActionId = Stage->strActionId;
					if (Source.eClockBasis == EFFECT_V2_CLOCK_BASIS::STAGE)
					{
						Candidate.iStartMs = StartMs - StageBox->iStartMs;
						Candidate.strClipOccurrenceId.clear();
					}
					else if (Source.eClockBasis == EFFECT_V2_CLOCK_BASIS::CLIP_OCCURRENCE)
					{
						const TIMELINE_ITEM* ClipBox = nullptr;
						for (const auto& Box : Items)
							if (Box.eLane == TIMELINE_LANE::ANIMATION && Box.eOwner == DETAIL_OWNER::ANIMATION &&
								Box.strPatternId == Pattern.strPatternId && Box.strStageId == Stage->strStageId &&
								StartMs >= Box.iStartMs && StartMs < Box.iEndMs &&
								(!ClipBox || Box.iStartMs > ClipBox->iStartMs)) ClipBox = &Box;
						if (!ClipBox)
						{ Status = "V2 Animation-clock drag must begin inside an Animation box; the binding was preserved."; return false; }
						const auto Clip = std::find_if(Stage->ClipOccurrences.begin(), Stage->ClipOccurrences.end(),
							[&](const auto& Row) { return Row.strClipOccurrenceId == ClipBox->strStableId; });
						if (Clip == Stage->ClipOccurrences.end() || !std::isfinite(Clip->fPlayRate) || Clip->fPlayRate <= 0.f ||
							(Source.eRepeatPolicy == EFFECT_V2_REPEAT_POLICY::EACH_LOOP && !Clip->bLoop))
						{ Status = "V2 drag cannot preserve its Animation rate/repeat policy at that target."; return false; }
						const double SourceMs = Clip->iSourceStartMs +
							static_cast<double>(StartMs - ClipBox->iStartMs) * Clip->fPlayRate;
						if (!std::isfinite(SourceMs) || SourceMs > 600000.0)
						{ Status = "V2 drag exceeds the source clock bound; the binding was preserved."; return false; }
						Candidate.iStartMs = static_cast<uint32_t>(std::llround(SourceMs));
						Candidate.strClipOccurrenceId = Clip->strClipOccurrenceId;
					}
					else { Status = "V2 drag clock basis is invalid."; return false; }
					OutBinding = std::move(Candidate);
					OutStage = &*Stage;
					return true;
				};
				std::string Status;
				const auto Snapshot = CEffectV2Catalog::Get().Get_Snapshot();
				const auto* Binding = Snapshot ? ResolveEffectV2Binding(*Snapshot,
					Item.strStableId, Item.strEffectV2ClipOccurrenceId) : nullptr;
				if (!Binding)
					Status = "V2 move lost its exact saved binding row. Load Effect Resources and select the box again.";
				else
				{
					const uint32_t RequestedStart = static_cast<uint32_t>((std::clamp)(
						static_cast<int64_t>(Item.iStartMs) + iWallDeltaMs, int64_t{0},
						static_cast<int64_t>(iRenderDurationMs)));
					EFFECT_V2_BINDING Candidate;
					const VALTAN_STAGE_VIEW* TargetStage = nullptr;
					if (ProjectV2Move(*pPattern, TimelineItems, *Binding, RequestedStart,
						Candidate, TargetStage, Status) && Validate_EffectV2BindingClock(*TargetStage, Candidate, Status))
					{
						if (Candidate.strStageId == Binding->strStageId && Candidate.strActionId == Binding->strActionId &&
							Candidate.strClipOccurrenceId == Binding->strClipOccurrenceId && Candidate.iStartMs == Binding->iStartMs)
							Status = "V2 Effect box start is unchanged.";
						else if (CEffectV2Catalog::Get().Stage_MoveBossValtanBinding(
							EFFECT_V2_STAGE_BINDING_KEY::From_Binding(*Binding), Candidate.strStageId,
							Candidate.strActionId, Candidate.strClipOccurrenceId, Candidate.iStartMs, Status))
						{
							m_iEffectV2CatalogRevision = CEffectV2Catalog::Get().Get_Revision();
							m_strSelectedStageId = Candidate.strStageId;
							m_strSelectedStableId = BuildEffectV2BindingStableId(Candidate, Candidate.strClipOccurrenceId);
							m_strEffectV2BindingEditId.clear();
							Refresh_EffectV2LocalPreviewAfterMutation(pPattern, Status);
						}
					}
				}
				m_strStatus = std::move(Status);
			}
			else if (nullptr != pEffectCue)
			{
				const uint32_t requested = static_cast<uint32_t>((std::clamp)(
					static_cast<int64_t>(Item.iStartMs) + iWallDeltaMs,
					int64_t{0}, static_cast<int64_t>(iRenderDurationMs)));
				std::string targetStageId, targetOccurrenceId, Status;
				if (MoveValtanEffectInvocation(m_pAnimationTool, m_pBalanceTool, *pPattern,
					*ItemStage, *pEffectCue, TimelineItems, requested, targetStageId, targetOccurrenceId, Status))
				{
					m_strSelectedStageId = targetStageId;
					m_strSelectedStableId = targetOccurrenceId;
					m_strEffectEditIdentity.clear();
					Invalidate_TimelineCache();
				}
				m_strStatus = std::move(Status);
			}
			else if (nullptr != pSoundCue)
			{
				const uint32_t startMs = static_cast<uint32_t>((std::clamp)(
					int64_t(Item.iStartMs) + iWallDeltaMs, int64_t{0}, int64_t{600000}));
				std::string Status;
				(void)Apply_SoundTimelinePlacement(*pPattern, *ItemStage, *pSoundCue, startMs,
					pSoundCue->bHasPlaybackOffset ? pSoundCue->iPlaybackOffsetMs : 0u,
					pSoundCue->bHasPlaybackDuration ? std::optional<uint32_t>{pSoundCue->iPlaybackDurationMs} : std::nullopt,
					Status);
				m_strStatus = m_strSoundStatus = std::move(Status);
			}
		}
		ImGui::PopID();
		}
		if (bLaneRowHovered && !bLaneBlockHovered && !m_bTimelineTrimActive &&
			!m_bTimelineMoveActive && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			m_bTimelineMarqueeActive = true;
			m_bTimelineMarqueeDragged = false;
			m_strTimelineMarqueePatternId = pPattern->strPatternId;
			m_fTimelineMarqueeAnchorX = ImGui::GetIO().MousePos.x - CanvasOrigin.x;
			m_fTimelineMarqueeAnchorY = ImGui::GetIO().MousePos.y - CanvasOrigin.y;
			m_TimelineMarqueeBefore = m_TimelineSelection;
			m_TimelineMarqueeFocus = {m_strSelectedPatternId, m_strSelectedStageId,
				m_strSelectedStableId, m_eDetailOwner};
			m_TimelineMarqueeBase = ImGui::GetIO().KeyShift ? m_TimelineSelection :
				std::vector<TIMELINE_SELECTION>{};
			if (ImGui::GetIO().KeyShift && m_TimelineMarqueeBase.empty())
			{
				const auto previous = std::find_if(TimelineItems.begin(), TimelineItems.end(), [&](const auto& row) {
					return row.strPatternId == m_strSelectedPatternId &&
						row.strStageId == m_strSelectedStageId && row.strStableId == m_strSelectedStableId && row.eOwner == m_eDetailOwner;
				});
				if (previous != TimelineItems.end()) m_TimelineMarqueeBase.push_back(Timeline_SelectionKey(*previous));
			}
		}
		const float fPlayheadX = RowMin.x +
			static_cast<float>((std::min)(m_iPlayheadMs, iRenderDurationMs)) *
			m_fTimelinePixelsPerSecond * 0.001f;
		pDrawList->AddLine(
			ImVec2(fPlayheadX, RowMin.y), ImVec2(fPlayheadX, RowMax.y),
			IM_COL32(255, 220, 72, 220), 1.5f);
		ImGui::PopID();
		fLaneY += fLaneHeight;
	}
	if (m_bTimelineMarqueeActive)
	{
		const bool cancel = ImGui::IsKeyPressed(ImGuiKey_Escape, false);
		const ImVec2 anchor(CanvasOrigin.x + m_fTimelineMarqueeAnchorX,
			CanvasOrigin.y + m_fTimelineMarqueeAnchorY);
		const ImVec2 pointer = ImGui::GetIO().MousePos;
		m_bTimelineMarqueeDragged |= ImGui::IsMouseDragging(ImGuiMouseButton_Left, 4.f);
		if (cancel)
			m_TimelineSelection = m_TimelineMarqueeBefore;
		else if (m_bTimelineMarqueeDragged || ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			m_TimelineSelection = m_TimelineMarqueeBase;
			if (m_bTimelineMarqueeDragged)
				for (const auto& box : MarqueeBoxes)
					if (CompositionTimeline::MarqueeIntersectsBox(anchor, pointer, box.min, box.max))
					{
						const auto key = Timeline_SelectionKey(TimelineItems[box.index]);
						if (std::find(m_TimelineSelection.begin(), m_TimelineSelection.end(), key) == m_TimelineSelection.end())
							m_TimelineSelection.push_back(key);
					}
		}
		if (m_bTimelineMarqueeDragged && !cancel)
		{
			const ImVec2 min((std::min)(anchor.x, pointer.x), (std::min)(anchor.y, pointer.y));
			const ImVec2 max((std::max)(anchor.x, pointer.x), (std::max)(anchor.y, pointer.y));
			pDrawList->AddRectFilled(min, max, IM_COL32(88, 160, 240, 40));
			pDrawList->AddRect(min, max, IM_COL32(110, 185, 255, 230));
		}
		if (cancel)
		{
			m_strSelectedStageId = m_TimelineMarqueeFocus.strStageId;
			m_strSelectedStableId = m_TimelineMarqueeFocus.strStableId;
			m_eDetailOwner = m_TimelineMarqueeFocus.eOwner;
		}
		else if (!m_TimelineSelection.empty())
		{
			const auto& focus = m_TimelineSelection.back();
			m_strSelectedStageId = focus.strStageId;
			m_strSelectedStableId = focus.strStableId;
			m_eDetailOwner = focus.eOwner;
		}
		else if (m_bTimelineMarqueeDragged || ImGui::IsMouseReleased(ImGuiMouseButton_Left))
		{
			m_strSelectedStableId.clear();
			m_eDetailOwner = DETAIL_OWNER::GAMEPLAY_STAGE;
		}
		if (cancel || !ImGui::IsMouseDown(ImGuiMouseButton_Left))
		{
			m_bTimelineMarqueeActive = false;
			m_TimelineMarqueeBefore.clear();
			m_TimelineMarqueeBase.clear();
		}
	}
	if (m_bTimelineTrimActive &&
		!ImGui::IsMouseDown(ImGuiMouseButton_Left))
	{
		m_bTimelineTrimActive = false;
		m_bTimelineTrimStartEdge = false;
		m_strTimelineTrimPatternId.clear();
		m_strTimelineTrimStageId.clear();
		m_strTimelineTrimStableId.clear();
		m_iTimelineTrimMouseStartMs = 0u;
		m_iTimelineTrimSourceEndpointMs = 0u;
	}
	if (m_bTimelineMoveActive &&
		!ImGui::IsMouseDown(ImGuiMouseButton_Left))
	{
		m_bTimelineMoveActive = false;
		m_strTimelineMovePatternId.clear();
		m_strTimelineMoveStageId.clear();
		m_strTimelineMoveStableId.clear();
		m_iTimelineMoveMouseStartMs = 0u;
		m_iTimelineMoveSourceStartMs = 0u;
		m_iTimelineMoveSourceEndMs = 0u;
	}
	ImGui::SetCursorScreenPos(CanvasOrigin);
	ImGui::Dummy(ImVec2(TIMELINE_LANE_LABEL_WIDTH + fCanvasWidth, fTimelineHeight + 2.f * TIMELINE_ROW_HEIGHT));
	ImGui::EndChild();
}

void Client::CValtanActionWorkbench::Render_WindowMenu()
{
	if (!ImGui::BeginMenuBar())
		return;
	if (ImGui::BeginMenu("Windows"))
	{
		ImGui::MenuItem("Patterns", nullptr, &m_bPatternsWindowVisible);
		ImGui::MenuItem("Preview", nullptr, &m_bPreviewWindowVisible);
		ImGui::MenuItem("Sequencer", nullptr, &m_bSequencerWindowVisible);
		ImGui::MenuItem("Box Detail", nullptr, &m_bDetailsWindowVisible);
		ImGui::MenuItem("Resources", nullptr, &m_bResourcesWindowVisible);
		ImGui::MenuItem("Boss Pattern", nullptr,
			&m_bBossPatternWindowVisible);
		ImGui::Separator();
		if (ImGui::MenuItem("Show All"))
		{
			m_bPatternsWindowVisible = true;
			m_bPreviewWindowVisible = true;
			m_bSequencerWindowVisible = true;
			m_bDetailsWindowVisible = true;
			m_bResourcesWindowVisible = true;
			m_bBossPatternWindowVisible = true;
		}
		if (ImGui::MenuItem("Reset Window Layout"))
		{
			m_bTimelineMaximized = false;
			m_bResetLayoutRequested = true;
		}
		ImGui::EndMenu();
	}
	ImGui::EndMenuBar();
}

bool_t Client::CValtanActionWorkbench::Render_SessionWindow(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const VALTAN_STAGE_VIEW* const pStage,
	const bool_t bPatternMutationAdmitted)
{
	if (!m_bSessionWindowVisible)
		return false;
	const ImGuiViewport* const pViewport = ImGui::GetMainViewport();
	const COMPOSITION_DEFAULT_LAYOUT Layout =
		BuildCompositionDefaultLayout(pViewport);
	const ImGuiCond Condition = m_bApplyResetLayoutThisFrame ?
		ImGuiCond_Always : ImGuiCond_FirstUseEver;
	ImGui::SetNextWindowPos(Layout.SessionPos, Condition);
	ImGui::SetNextWindowSize(Layout.SessionSize, Condition);
	if (!ImGui::Begin(
			"Server Replay###CompositionSessionWindow",
			&m_bSessionWindowVisible, ImGuiWindowFlags_MenuBar))
	{
		Render_WindowMenu();
		ImGui::End();
		return false;
	}
	Render_WindowMenu();
	if (Render_Toolbar(pPattern, bPatternMutationAdmitted))
	{
		ImGui::End();
		return true;
	}
	ImGui::End();
	return false;
}

void Client::CValtanActionWorkbench::Render_PatternsWindow(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const VALTAN_STAGE_VIEW* const,
	const bool_t bPatternMutationAdmitted)
{
	if (!m_bPatternsWindowVisible)
		return;
	const ImGuiViewport* const pViewport = ImGui::GetMainViewport();
	const COMPOSITION_DEFAULT_LAYOUT Layout =
		BuildCompositionDefaultLayout(pViewport);
	const ImGuiCond Condition = m_bApplyResetLayoutThisFrame ?
		ImGuiCond_Always : ImGuiCond_FirstUseEver;
	if (m_iRequestedPatternTab >= 0)
	{
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always);
		ImGui::SetNextWindowFocus();
	}
	ImGui::SetNextWindowPos(Layout.PatternsPos, Condition);
	ImGui::SetNextWindowSize(Layout.PatternsSize, Condition);
	const bool_t bWindowVisible = ImGui::Begin(
			"Composition Actions###CompositionPatternsWindow",
			&m_bPatternsWindowVisible, ImGuiWindowFlags_MenuBar);
	Render_PendingPatternSelectionModal();
	if (!bWindowVisible)
	{
		Render_WindowMenu();
		ImGui::End();
		return;
	}
	Render_WindowMenu();
	Render_PatternsPane(pPattern, nullptr, bPatternMutationAdmitted);
	ImGui::End();
}

void Client::CValtanActionWorkbench::Render_PatternsPane(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const VALTAN_STAGE_VIEW* const,
	const bool_t bPatternMutationAdmitted)
{
	ImGui::TextDisabled("Canonical admission: %s", Admission_Label());
	ImGui::SameLine();
	if (ImGui::SmallButton("Reload Canonical"))
		m_bWorkbenchReloadRequested = true;
	if (!m_strStatus.empty())
		ImGui::TextWrapped("Canonical status: %s", m_strStatus.c_str());
	ImGui::Separator();
	if (ImGui::BeginTabBar("##CompositionPatternDomainTabs"))
	{
		if (ImGui::BeginTabItem(
				"Patterns / Stages", nullptr,
				0 == m_iRequestedPatternTab ?
					ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
		{
			Render_Browser(pPattern);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem(
				"Create New Pattern", nullptr,
				1 == m_iRequestedPatternTab ?
					ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
		{
			ImGui::BeginDisabled(!bPatternMutationAdmitted);
			if (nullptr != m_pAnimationTool)
				m_pAnimationTool->Render_ValtanCompositionPatternCreator();
			else
				ImGui::TextDisabled("Animation Intake owner is unavailable.");
			ImGui::EndDisabled();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
		m_iRequestedPatternTab = -1;
	}
}

void Client::CValtanActionWorkbench::Render_PreviewWindow(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const bool_t bLocalPreviewAdmitted,
	const bool_t bServerVerificationAdmitted)
{
	if (!m_bPreviewWindowVisible)
		return;
	const ImGuiViewport* const pViewport = ImGui::GetMainViewport();
	const COMPOSITION_DEFAULT_LAYOUT Layout =
		BuildCompositionDefaultLayout(pViewport);
	const ImGuiCond Condition = m_bApplyResetLayoutThisFrame ?
		ImGuiCond_Always : ImGuiCond_FirstUseEver;
	ImGui::SetNextWindowPos(Layout.PreviewPos, Condition);
	ImGui::SetNextWindowSize(Layout.PreviewSize, Condition);
	if (ImGui::Begin(
			"Composition Preview###CompositionPreviewWindow",
			&m_bPreviewWindowVisible, ImGuiWindowFlags_MenuBar))
	{
		Render_WindowMenu();
		Render_Preview(
			pPattern, bLocalPreviewAdmitted, bServerVerificationAdmitted);
	}
	else
	{
		Render_WindowMenu();
	}
	ImGui::End();
}

void Client::CValtanActionWorkbench::Render_SequencerWindow(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const bool_t bLocalPreviewAdmitted,
	const bool_t bMutationAdmitted,
	const bool_t bPatternMutationAdmitted)
{
	if (!m_bSequencerWindowVisible)
	{
		m_bTimelineMaximized = false;
		return;
	}
	const ImGuiViewport* const pViewport = ImGui::GetMainViewport();
	const COMPOSITION_DEFAULT_LAYOUT Layout =
		BuildCompositionDefaultLayout(pViewport);
	const ImGuiCond Condition = m_bApplyResetLayoutThisFrame ?
		ImGuiCond_Always : ImGuiCond_FirstUseEver;
	const char_t* pWindowTitle =
		"Composition Sequencer###CompositionSequencerWindowResizableV3";
	ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_MenuBar;
	if (m_bTimelineMaximized && nullptr != pViewport)
	{
		pWindowTitle =
			"Composition Timeline###CompositionTimelineMaximizedOverlay";
		WindowFlags |= ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoSavedSettings;
		const ImVec2 MaximizedPos(
			pViewport->WorkPos.x + pViewport->WorkSize.x * 0.02f,
			pViewport->WorkPos.y + pViewport->WorkSize.y * 0.02f);
		const ImVec2 MaximizedSize(
			pViewport->WorkSize.x * 0.96f,
			pViewport->WorkSize.y * 0.96f);
		ImGui::SetNextWindowPos(MaximizedPos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(MaximizedSize, ImGuiCond_Always);
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always);
	}
	else
	{
		ImGui::SetNextWindowPos(Layout.SequencerPos, Condition);
		ImGui::SetNextWindowSize(Layout.SequencerSize, Condition);
	}
	bool_t bMaximizedOverlayOpen = true;
	bool_t* const pWindowOpen = m_bTimelineMaximized ?
		&bMaximizedOverlayOpen : &m_bSequencerWindowVisible;
	const bool_t bWindowVisible = ImGui::Begin(
		pWindowTitle, pWindowOpen, WindowFlags);
	if (bWindowVisible)
	{
		Render_WindowMenu();
		Render_Timeline(
			pPattern, bLocalPreviewAdmitted,
			bMutationAdmitted, bPatternMutationAdmitted);
	}
	else
	{
		Render_WindowMenu();
	}
	ImGui::End();
	if (m_bTimelineMaximized && !bMaximizedOverlayOpen)
		m_bTimelineMaximized = false;
}

void Client::CValtanActionWorkbench::Render_DetailsWindow(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const VALTAN_STAGE_VIEW* const pStage,
	const bool_t bMutationAdmitted,
	const bool_t bPatternMutationAdmitted)
{
	if (!m_bDetailsWindowVisible)
		return;
	const ImGuiViewport* const pViewport = ImGui::GetMainViewport();
	const COMPOSITION_DEFAULT_LAYOUT Layout =
		BuildCompositionDefaultLayout(pViewport);
	const ImGuiCond Condition = m_bApplyResetLayoutThisFrame ?
		ImGuiCond_Always : ImGuiCond_FirstUseEver;
	ImGui::SetNextWindowPos(Layout.DetailsPos, Condition);
	ImGui::SetNextWindowSize(Layout.DetailsSize, Condition);
	if (ImGui::Begin(
			"Box Detail###CompositionDetailsWindow",
			&m_bDetailsWindowVisible, ImGuiWindowFlags_MenuBar))
	{
		Render_WindowMenu();
		Render_Details(
			pPattern, pStage, bMutationAdmitted, bPatternMutationAdmitted);
	}
	else
	{
		Render_WindowMenu();
	}
	ImGui::End();
}

void Client::CValtanActionWorkbench::Request_LaneAuthoring(
	const TIMELINE_LANE eLane,
	const VALTAN_PATTERN_VIEW& Pattern,
	const VALTAN_STAGE_VIEW& Stage)
{
	const auto OpenResources = [this, &Pattern, &Stage](
		const RESOURCE_DOMAIN eDomain)
	{
		m_eRequestedResourceDomain = eDomain;
		m_bResourceDomainSelectionRequested = true;
		m_bResourcesWindowVisible = true;
		m_bResourcesWindowExpandRequested = true;
		m_bResourcesWindowFocusRequested = true;
		m_strResourceTargetPatternId = Pattern.strPatternId;
		m_strResourceTargetStageId = Stage.strStageId;
		const auto* selectedClip = ResolveEffectAppendClip(Stage, {}, m_strSelectedStableId,
			ResolveAppendCursorClipId(m_TimelineItems, Pattern.strPatternId, Stage.strStageId, m_iPlayheadMs));
		m_strResourceTargetClipOccurrenceId = selectedClip ? selectedClip->strClipOccurrenceId : std::string{};
		if (RESOURCE_DOMAIN::EFFECT == eDomain)
			m_strEffectAddClipOccurrenceId =
				m_strResourceTargetClipOccurrenceId;
		if (RESOURCE_DOMAIN::SOUND == eDomain)
		{
			m_strSoundAddClipOccurrenceId =
				m_strResourceTargetClipOccurrenceId;
			const auto SoundClip = std::find_if(
				Stage.ClipOccurrences.begin(), Stage.ClipOccurrences.end(),
				[this](const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
				{
					return Clip.strClipOccurrenceId ==
						m_strResourceTargetClipOccurrenceId;
				});
			m_iSoundAddStartMs = Stage.ClipOccurrences.end() == SoundClip ?
				0u : SoundClip->iSourceStartMs;
			m_eSoundAddRepeatPolicy =
				VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
		}
	};

	switch (eLane)
	{
	case TIMELINE_LANE::STAGE:
		m_bDetailsWindowVisible = true;
		Select_Stage(
			Pattern, Stage, DETAIL_OWNER::GAMEPLAY_STAGE,
			Stage.strStageId + "/topology");
		m_strStatus = Pattern.bManualServerAudition ?
			"Stage authoring opened. Insert ACTIVE/WINDUP/GROGGY or an explicit WAIT gap after the selected Stage." :
			"This canonical Pattern exposes Stage timing as read-only; Create New Pattern produces a manual authoring topology.";
		break;
	case TIMELINE_LANE::ANIMATION:
		OpenResources(RESOURCE_DOMAIN::ANIMATION);
		Select_Stage(
			Pattern, Stage, DETAIL_OWNER::ANIMATION,
			Stage.ClipOccurrences.empty() ? Stage.strStageId :
				Stage.ClipOccurrences.front().strClipOccurrenceId);
		m_strStatus =
			"Animation catalog opened. Select a semantic Sequence, then replace, append, or duplicate the selected Stage slots.";
		break;
	case TIMELINE_LANE::EFFECT:
		OpenResources(RESOURCE_DOMAIN::EFFECT);
		Select_Stage(Pattern, Stage, DETAIL_OWNER::EFFECT, Stage.strStageId);
		m_strEffectAddClipOccurrenceId =
			m_strResourceTargetClipOccurrenceId;
		m_strStatus =
			"Effect catalog and typed invocation Details opened for the selected Stage.";
		break;
	case TIMELINE_LANE::SOUND:
		OpenResources(RESOURCE_DOMAIN::SOUND);
		Select_Stage(Pattern, Stage, DETAIL_OWNER::SOUND, Stage.strStageId);
		m_strSoundAddClipOccurrenceId =
			m_strResourceTargetClipOccurrenceId;
		if (const auto TargetClip = std::find_if(
				Stage.ClipOccurrences.begin(), Stage.ClipOccurrences.end(),
				[this](const VALTAN_CLIP_OCCURRENCE_VIEW& Clip)
				{
					return Clip.strClipOccurrenceId ==
						m_strSoundAddClipOccurrenceId;
				}); TargetClip != Stage.ClipOccurrences.end())
		{
			m_iSoundAddStartMs = TargetClip->iSourceStartMs;
		}
		m_strStatus =
			"Sound event catalog and typed Pattern Sound row editor opened for the selected Stage.";
		break;
	case TIMELINE_LANE::LOGIC:
		OpenResources(RESOURCE_DOMAIN::LOGIC);
		break;
	case TIMELINE_LANE::SUMMON:
		OpenResources(RESOURCE_DOMAIN::SUMMON);
		break;
	case TIMELINE_LANE::SCENE_PROFILE:
		OpenResources(RESOURCE_DOMAIN::SCENE_PROFILE);
		break;
	case TIMELINE_LANE::LIGHT:
		OpenResources(RESOURCE_DOMAIN::LIGHT);
		break;
	case TIMELINE_LANE::COLLIDER:
		m_bDetailsWindowVisible = true;
		Select_Stage(
			Pattern, Stage, DETAIL_OWNER::GAMEPLAY_STAGE,
			Stage.strStageId + "/collider");
		if (nullptr == m_pBalanceTool)
		{
			m_strStatus =
				"Server Collider Details opened read-only because the typed Pattern draft owner is unavailable.";
			break;
		}
		{
			CBalanceTool::PATTERN_STAGE_EDIT Draft;
			std::string Status;
			if (!m_pBalanceTool->Get_ValtanStageDraft(
					Pattern.strPatternId, Stage.strStageId, Draft, Status))
			{
				m_strStatus = std::move(Status);
				break;
			}
			if ("NONE" != Draft.hitShape)
			{
				m_strStatus =
					"Server Collider geometry and timing Details opened; visual Effect geometry is never copied into hit authority.";
				break;
			}
			const std::vector<std::string> DamageProfiles =
				m_pBalanceTool->Get_ValtanDamageProfileIds();
			if (!InitializeDefaultValtanColliderBoxDraft(
					Draft, DamageProfiles, m_iDamageProfileSelection, Status) ||
				!SetValtanStageDraftWithSoundDependencyAdmission(
					m_pAnimationTool, m_pBalanceTool,
					m_bPatternShakesReady ? &m_PatternShakes : nullptr,
					Pattern, Stage, Draft, Status))
			{
				m_strStatus = std::move(Status);
				break;
			}
			Refresh_PatternLocalPreviewAfterMutation(&Pattern, Status);
			m_strStatus = std::move(Status);
			Invalidate_TimelineCache();
		}
		break;
	case TIMELINE_LANE::WORLD:
		OpenResources(RESOURCE_DOMAIN::WORLD);
		break;
	case TIMELINE_LANE::CAMERA:
		OpenResources(RESOURCE_DOMAIN::CAMERA);
		if (!Stage.CameraInvocations.empty())
		{
			Select_Stage(
				Pattern, Stage, DETAIL_OWNER::CAMERA,
				Stage.CameraInvocations.front().strCameraInvocationId);
		}
		else
		{
			m_bCameraToolOpenRequested = true;
			m_strStatus =
				"No exact Camera invocation is admitted on this Stage; opening the Camera owner tool.";
		}
		break;
	default:
		break;
	}
}

void Client::CValtanActionWorkbench::Render_ResourcesWindow(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const VALTAN_STAGE_VIEW* const pStage,
	const bool_t bMutationAdmitted,
	const bool_t bPatternMutationAdmitted)
{
	if (!m_bResourcesWindowVisible)
		return;
	const ImGuiViewport* const pViewport = ImGui::GetMainViewport();
	const COMPOSITION_DEFAULT_LAYOUT Layout =
		BuildCompositionDefaultLayout(pViewport);
	const ImGuiCond Condition = m_bApplyResetLayoutThisFrame ?
		ImGuiCond_Always : ImGuiCond_FirstUseEver;
	if (m_bResourcesWindowExpandRequested && nullptr != pViewport)
	{
		const ImVec2 ExpandedPos(
			pViewport->WorkPos.x + pViewport->WorkSize.x * 0.08f,
			pViewport->WorkPos.y + pViewport->WorkSize.y * 0.12f);
		const ImVec2 ExpandedSize(
			pViewport->WorkSize.x * 0.62f,
			pViewport->WorkSize.y * 0.72f);
		ImGui::SetNextWindowPos(ExpandedPos, ImGuiCond_Always);
		ImGui::SetNextWindowSize(ExpandedSize, ImGuiCond_Always);
		ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always);
	}
	else
	{
		ImGui::SetNextWindowPos(Layout.ResourcesPos, Condition);
		ImGui::SetNextWindowSize(Layout.ResourcesSize, Condition);
	}
	if (m_bResourcesWindowFocusRequested)
		ImGui::SetNextWindowFocus();
	m_bResourcesWindowExpandRequested = false;
	m_bResourcesWindowFocusRequested = false;
	if (!ImGui::Begin(
			"Composition Resources###CompositionResourcesWindowResizableV2",
			&m_bResourcesWindowVisible, ImGuiWindowFlags_MenuBar))
	{
		Render_WindowMenu();
		ImGui::End();
		return;
	}
	Render_WindowMenu();
	Render_ResourcesPane(pPattern, pStage, bMutationAdmitted, bPatternMutationAdmitted);
	ImGui::End();
}

void Client::CValtanActionWorkbench::Render_SupplementalResources(
	const RESOURCE_DOMAIN domain, const VALTAN_PATTERN_VIEW* pattern,
	const VALTAN_STAGE_VIEW* stage, const bool editable)
{
	if (domain == RESOURCE_DOMAIN::PATTERN)
	{
		Render_Browser(pattern);
		return;
	}
	const bool canAppend = editable && pattern && stage && m_pBalanceTool;
	uint32_t stageStart = 0u;
	if (stage)
		for (const auto& item : m_TimelineItems)
			if (item.eLane == TIMELINE_LANE::STAGE && item.strStageId == stage->strStageId)
			{ stageStart = item.iStartMs; break; }
	const uint32_t start = !stage ? 0u : (std::min)(
		m_iPlayheadMs > stageStart ? m_iPlayheadMs - stageStart : 0u,
		stage->iDurationMs ? stage->iDurationMs - 1u : 0u);
	const auto changed = [this, pattern](std::string& status)
	{
		// 모든 pane이 반환할 때까지 불변 frame view의 저장소를 유지한다.
		Invalidate_TimelineCache();
		Refresh_PatternLocalPreviewAfterMutation(pattern, status);
		m_strStatus = status;
	};
	if (domain == RESOURCE_DOMAIN::SCENE_PROFILE || domain == RESOURCE_DOMAIN::LIGHT)
	{
		const auto& resources = domain == RESOURCE_DOMAIN::SCENE_PROFILE ? m_SceneProfileResourceIds : m_LightResourceIds;
		if (resources.empty()) ImGui::TextDisabled("No resources are loaded for this category.");
		for (const auto& id : resources)
		{
			ImGui::PushID(id.c_str());
			ImGui::TextWrapped("%s", id.c_str());
			ImGui::BeginDisabled(!canAppend);
			if (ImGui::SmallButton("Add to Stage"))
			{
				std::string status;
				if (domain == RESOURCE_DOMAIN::SCENE_PROFILE)
				{
					auto cues = stage->SceneProfileOccurrences;
					BOSS_STAGE_SCENE_PROFILE_OCCURRENCE cue;
					for (uint32_t ordinal = 1u;; ++ordinal)
					{
						cue.strOccurrenceId = pattern->strPatternId + "." + stage->strStageId + ".scene." + std::to_string(ordinal);
						if (std::none_of(cues.begin(), cues.end(), [&](const auto& x) { return x.strOccurrenceId == cue.strOccurrenceId; })) break;
					}
					cue.strProfileId = id; cue.iStartMs = start; cue.iDurationMs = stage->iDurationMs - start;
					cues.push_back(cue);
					if (m_pBalanceTool->Set_ValtanStageSceneProfileOccurrences(pattern->strPatternId, stage->strStageId, cues, status))
					{
						Select_Stage(*pattern, *stage, DETAIL_OWNER::SCENE_PROFILE, cue.strOccurrenceId);
						changed(status);
					}
				}
				else
				{
					auto cues = stage->LightOccurrences;
					BOSS_STAGE_LIGHT_OCCURRENCE cue;
					for (uint32_t ordinal = 1u;; ++ordinal)
					{
						cue.strOccurrenceId = pattern->strPatternId + "." + stage->strStageId + ".light." + std::to_string(ordinal);
						if (std::none_of(cues.begin(), cues.end(), [&](const auto& x) { return x.strOccurrenceId == cue.strOccurrenceId; })) break;
					}
					cue.strLightResourceId = id; cue.iStartMs = start; cue.iDurationMs = stage->iDurationMs - start;
					cues.push_back(cue);
					if (m_pBalanceTool->Set_ValtanStageLightOccurrences(pattern->strPatternId, stage->strStageId, cues, status))
					{
						Select_Stage(*pattern, *stage, DETAIL_OWNER::LIGHT, cue.strOccurrenceId);
						changed(status);
					}
				}
				m_strStatus = status;
			}
			ImGui::EndDisabled(); ImGui::PopID();
		}
		return;
	}
	if (domain == RESOURCE_DOMAIN::COLLIDER)
	{
		ImGui::TextWrapped("Add or edit the selected Stage's hit area.");
		ImGui::BeginDisabled(!canAppend);
		if (ImGui::Button("Add / Edit Collider")) Request_LaneAuthoring(TIMELINE_LANE::COLLIDER, *pattern, *stage);
		ImGui::EndDisabled();
		return;
	}
	if (domain == RESOURCE_DOMAIN::CAMERA)
	{
		if (ImGui::Button("Open Camera Tool")) m_bCameraToolOpenRequested = true;
		std::set<std::string> seen;
		for (const auto* source : Collect_Patterns())
			for (const auto& sourceStage : source->Stages)
				for (const auto& camera : sourceStage.CameraInvocations)
				{
					if (!seen.insert(camera.strCameraCueId).second) continue;
					ImGui::PushID(camera.strCameraCueId.c_str());
					ImGui::TextWrapped("%s", camera.strCameraCueId.c_str());
					ImGui::BeginDisabled(!canAppend);
					if (ImGui::SmallButton("Add to Stage"))
					{
						auto cue = camera;
						for (uint32_t ordinal = 1u;; ++ordinal)
						{
							cue.strCameraInvocationId = pattern->strPatternId + "." + stage->strStageId + ".camera." + std::to_string(ordinal);
							if (std::none_of(stage->CameraInvocations.begin(), stage->CameraInvocations.end(), [&](const auto& x) { return x.strCameraInvocationId == cue.strCameraInvocationId; })) break;
						}
						cue.strTrigger = "ENTER"; cue.iStartOffsetMs = start;
						cue.strDurationPolicy = "EXPLICIT";
						cue.iDurationMs = (std::min)(cue.iDurationMs, stage->iDurationMs - start);
						std::string status;
						if (m_pBalanceTool->Upsert_ValtanCameraInvocationDraft(pattern->strPatternId, stage->strStageId, cue, status))
						{
							Select_Stage(*pattern, *stage, DETAIL_OWNER::CAMERA, cue.strCameraInvocationId);
							changed(status);
						}
						m_strStatus = status;
					}
					ImGui::EndDisabled(); ImGui::PopID();
				}
		return;
	}
	if (domain == RESOURCE_DOMAIN::SUMMON)
	{
		ImGui::SetNextItemWidth(-1.f);
		ImGui::InputTextWithHint("##ValtanSummonSearch", "Search Summon, Pattern or Effect...",
			m_SummonResourceSearch.data(), m_SummonResourceSearch.size());
		std::set<std::string> seen;
		for (const auto* source : Collect_Patterns())
			for (const auto& sourceStage : source->Stages)
				for (const auto& object : sourceStage.CombatObjectEffects)
				{
					const std::string search = object.strCombatObjectArchetypeId + " " + object.strEffectAssetId + " " +
						object.strEffectV2GroupId + " " + source->strPatternId + " " + source->strDisplayName;
					if (!ContainsInsensitive(search, m_SummonResourceSearch.data()) ||
						!seen.insert(object.strCombatObjectArchetypeId).second) continue;
					ImGui::PushID(object.strCombatObjectArchetypeId.c_str());
					ImGui::TextWrapped("%s | %s", source->strDisplayName.c_str(), object.strCombatObjectArchetypeId.c_str());
					ImGui::TextDisabled("Spawn count %u | lifetime %u ms", object.iSpawnValue, object.iLifetimeMs);
					const auto capture = [&, source, object](std::string& message) -> COMPOSITION_TRANSFER {
						auto value = std::make_shared<VALTAN_EDIT_TRANSFER>(); value->owner = DETAIL_OWNER::COMBAT_OBJECT;
						value->summon = object; value->patternId = source->strPatternId; value->stageId = sourceStage.strStageId;
						value->sourceRevision = m_strPinnedAuthoringSourceRevision; value->label = object.strCombatObjectArchetypeId;
						message = "Copied the reusable Summon definition reference."; return value;
					};
					if (ImGui::SmallButton("Copy Resource")) CCompositionClipboard::Get().Write(capture(m_strStatus));
					// 드래그 시작은 ID가 있는 ImGui item 뒤에 배치해야 한다. 설명용 Text 행 뒤에 붙이지 않는다.
					Offer_CompositionResourceDrag(object.strCombatObjectArchetypeId.c_str(), [&]() { return capture(m_strStatus); });
					ImGui::BeginDisabled(!canAppend);
					if (ImGui::SmallButton("Copy to Stage"))
					{
						auto cue = object;
						std::string status;
						VALTAN_PATTERN_TREE_VIEW fresh;
						const bool ready = m_pBalanceTool->Get_ValtanAuthoringView(fresh, status);
						// 수명은 공유 resource가 소유한다. 오래된 library 투영을 Append할 때
						// resource의 미저장 편집을 이전 값으로 되돌리지 않는다.
						if (ready)
							for (const auto* group : { &fresh.Gimmicks, &fresh.Rotation })
								for (const auto& owner : *group)
									for (const auto& currentStage : owner.Stages)
										for (const auto& current : currentStage.CombatObjectEffects)
											if (current.strCombatObjectArchetypeId == cue.strCombatObjectArchetypeId) cue = current;
						VALTAN_COMBAT_OBJECT_EFFECT_VIEW created;
						if (ready && m_pBalanceTool->Clone_ValtanSummonDraft(pattern->strPatternId, stage->strStageId, cue,
							m_strPinnedAuthoringSourceRevision, start, created, status))
						{
							Select_Stage(*pattern, *stage, DETAIL_OWNER::COMBAT_OBJECT, created.strCombatObjectArchetypeId);
							changed(status);
						}
						m_strStatus = status;
					}
					ImGui::EndDisabled(); ImGui::PopID();
				}
		return;
	}
	if (domain == RESOURCE_DOMAIN::LOGIC)
	{
		ImGui::BeginDisabled(!canAppend);
		if (ImGui::Button("Counter Window"))
			Select_Stage(*pattern, *stage, DETAIL_OWNER::GAMEPLAY_STAGE, BuildBranchStableId(stage->strStageId, "COUNTER_HIT"));
		ImGui::EndDisabled();
	}
	std::set<std::string> seen;
	for (const auto* source : Collect_Patterns())
		for (const auto& sourceStage : source->Stages)
			for (const auto& action : sourceStage.Actions)
			{
				const bool spawn = action.strKind.find("SPAWN_COMBAT_OBJECT") == 0u;
				const bool world = action.strKind == "TRIGGER_WORLD_EVENT_SET";
				if ((domain == RESOURCE_DOMAIN::SUMMON) != spawn || (domain == RESOURCE_DOMAIN::WORLD) != world) continue;
				const std::string key = action.strKind + "/" + action.strTargetId + "/" + action.strTrigger;
				if (!seen.insert(key).second) continue;
				ImGui::PushID(key.c_str());
				ImGui::TextWrapped("%s | %s [%s]", action.strKind.c_str(), action.strTargetId.c_str(), action.strTrigger.c_str());
				if (!world && !spawn)
				{
					const auto capture = [&, action](std::string& message) -> COMPOSITION_TRANSFER {
						auto value = std::make_shared<VALTAN_EDIT_TRANSFER>(); value->owner = DETAIL_OWNER::GAMEPLAY_STAGE;
						value->actions.push_back(action); value->label = key;
						if (action.strKind.starts_with("SET_")) for (const auto& other : sourceStage.Actions)
							if (other.strKind == action.strKind && other.strTargetId == action.strTargetId && other.strTrigger != action.strTrigger) value->actions.push_back(other);
						message = "Copied the typed Logic action and its paired release."; return value;
					};
					if (ImGui::SmallButton("Copy Resource")) CCompositionClipboard::Get().Write(capture(m_strStatus));
					// 드래그 시작은 ID가 있는 ImGui item 뒤에 배치해야 한다. 설명용 Text 행 뒤에 붙이지 않는다.
					Offer_CompositionResourceDrag(key.c_str(), [&]() { return capture(m_strStatus); });
				}
				ImGui::BeginDisabled(!canAppend);
				if (ImGui::SmallButton(world ? "Move to Stage" : "Add to Stage"))
				{
					std::string status;
					const bool applied = world ? m_pBalanceTool->Apply_ValtanCompositionDraftTransaction([&](std::string& diagnostic)
					{
						if (source->strPatternId == pattern->strPatternId && sourceStage.strStageId == stage->strStageId) return true;
						return m_pBalanceTool->Remove_ValtanStageActionDraft(source->strPatternId, sourceStage.strStageId, action, diagnostic) &&
							m_pBalanceTool->Upsert_ValtanStageActionDraft(pattern->strPatternId, stage->strStageId, action, diagnostic);
					}, status) : m_pBalanceTool->Upsert_ValtanStageActionDraft(pattern->strPatternId, stage->strStageId, action, status);
					if (applied)
					{
						Select_Stage(*pattern, *stage, DETAIL_OWNER::GAMEPLAY_STAGE, stage->strStageId + "/action/" + key);
						changed(status);
					}
					m_strStatus = status;
				}
				ImGui::EndDisabled(); ImGui::PopID();
			}
	if (domain == RESOURCE_DOMAIN::SUMMON || domain == RESOURCE_DOMAIN::WORLD)
		if (ImGui::CollapsingHeader("Existing occurrences")) Render_WorldObjectResources();
}

bool_t Client::CValtanActionWorkbench::Append_EffectResourceAtPatternEnd(
	const std::string& sourcePatternId, std::string& status)
{
	const std::string patternId = sourcePatternId;
	if (!m_pBalanceTool || m_strEffectAddAssetId.empty())
	{ status = "Select a Pattern and a saved Effect before Append."; return false; }
	const bool v1 = m_eEffectAddResourceKind == EFFECT_RESOURCE_KIND::V1_PATTERN;
	uint32_t durationMs = 1000u;
	if (v1)
	{
		const auto document = CEffectCatalog::Find(m_strEffectAddAssetId);
		if (!document) { status = "Effect Append rejected: the saved Effect resource is unavailable."; return false; }
		const auto span = ResolveEffectDocumentSpanMs(*document);
		if (span > 0u) durationMs = span;
	}
	else
	{
		const auto snapshot = CEffectV2Catalog::Get().Get_Snapshot();
		if (!snapshot || !snapshot->Is_Ready() || !snapshot->Can_MutateBossValtanBindings())
		{ status = "Effect Append requires the editable V2 resource catalog."; return false; }
		EFFECT_V2_BINDING resource;
		if (m_eEffectAddResourceKind == EFFECT_RESOURCE_KIND::V2_GROUP) resource.strGroupId = m_strEffectAddAssetId;
		else resource.strEffectId = m_strEffectAddAssetId;
		const auto span = ResolveEffectV2ResourceSpanMs(*snapshot, resource);
		if (!span.bResolved) { status = "Effect Append rejected: the V2 resource lifetime is invalid."; return false; }
		if (!span.bUnbounded && span.fMilliseconds > 0.0)
			durationMs = static_cast<uint32_t>((std::min)(600000.0, (std::max)(1.0, std::ceil(span.fMilliseconds))));
	}
	VALTAN_PATTERN_VIEW candidate;
	std::string stageId, actionId, occurrenceId;
	uint64_t endMs = 0u;
	uint32_t cueStartMs = 0u;
	const bool applied = CEffectV2Catalog::Get().Apply_BossValtanBindingDraftTransaction([&](std::string& effectStatus)
	{
		return m_pBalanceTool->Apply_ValtanCompositionDraftTransaction([&](std::string& diagnostic)
		{
			if (!m_pBalanceTool->Get_ValtanPatternDraft(patternId, candidate, diagnostic)) return false;
			if (candidate.Stages.empty()) { diagnostic = "Effect Append requires the Pattern's empty seed Stage."; return false; }
			for (const auto& stage : candidate.Stages) endMs += stage.iDurationMs;
			if (candidate.bManualServerAudition)
			{
				if (!m_pBalanceTool->Can_Edit_ValtanManualStageTopology(patternId, diagnostic)) return false;
				uint64_t hash = 14695981039346656037ull;
				for (const unsigned char ch : patternId) hash = (hash ^ ch) * 1099511628211ull;
				for (uint32_t ordinal = 1u; ordinal <= 9999u; ++ordinal)
				{
					const std::string proposed = "EFFECT_APPEND_" + std::to_string(ordinal);
					const std::string action = "valtan.effect.append." + std::to_string(hash) + ".stage." + std::to_string(ordinal);
					if (std::none_of(candidate.Stages.begin(), candidate.Stages.end(), [&](const auto& stage)
						{ return stage.strStageId == proposed || stage.strActionId == action; }))
					{ stageId = proposed; actionId = action; break; }
				}
				if (stageId.empty()) { diagnostic = "Effect Append exhausted Stage identities."; return false; }
				const std::string afterStage = candidate.Stages.back().strStageId;
				if (!m_pBalanceTool->Insert_ValtanManualStageAfter(patternId, afterStage, stageId, actionId,
					"ACTIVE", durationMs, diagnostic)) return false;
			}
			else
			{
				// canonical gameplay의 기존 Stage graph를 유지한다.
				// 마지막 clock만 늘리고 이전 끝 뒤에 독립 Effect를 배치한다.
				const auto terminal = candidate.Stages.back();
				CBalanceTool::PATTERN_STAGE_EDIT draft;
				if (!m_pBalanceTool->Get_ValtanStageDraft(patternId, terminal.strStageId, draft, diagnostic)) return false;
				if (uint64_t(draft.durationMs) + durationMs > 600000u)
				{ diagnostic = "Effect Append would exceed the 600-second Stage clock."; return false; }
				stageId = terminal.strStageId; actionId = terminal.strActionId;
				cueStartMs = draft.durationMs; draft.durationMs += durationMs;
				if (!draft.animationSlots.empty() && draft.animationEndPolicy == "EXACT")
					draft.animationEndPolicy = "HOLD_LAST_POSE";
				if (!SetValtanStageDraftWithSoundDependencyAdmission(m_pAnimationTool, m_pBalanceTool,
					m_bPatternShakesReady ? &m_PatternShakes : nullptr, candidate, terminal, draft, diagnostic)) return false;
			}
			if (!m_pBalanceTool->Get_ValtanPatternDraft(patternId, candidate, diagnostic)) return false;
			const auto stage = std::find_if(candidate.Stages.begin(), candidate.Stages.end(),
				[&](const auto& row) { return row.strStageId == stageId; });
			if (stage == candidate.Stages.end()) { diagnostic = "Effect Append could not resolve the new Stage."; return false; }
			if (v1)
			{
				VALTAN_PRODUCT_EFFECT_CUE_VIEW cue;
				cue.strBindingId = BuildNextCompositionEffectCueId(candidate, *stage);
				cue.strOccurrenceId = cue.strBindingId + ".occurrence.01";
				cue.strPatternId = patternId; cue.strStageId = stageId; cue.strActionId = actionId;
				cue.bUsesStageClock = true; cue.iStageOffsetMs = cue.iSourceStartMs = cueStartMs;
				cue.strEffectAssetId = m_strEffectAddAssetId; cue.strAnchorSlotId = "root";
				cue.eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW; cue.strFollowPolicy = "follow";
				cue.eStopPolicy = EFFECT_STOP_POLICY::NATURAL; cue.strStopPolicy = "natural";
				cue.strRepeatPolicy = "once"; cue.bHasExplicitScalePolicy = true;
				cue.eScalePolicy = VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE; cue.strScalePolicy = "OWNER_RELATIVE";
				cue.iStageDurationMs = stage->iDurationMs;
				if (cue.strBindingId.empty() ||
					!ValidateValtanEffectInvocationClock(m_pAnimationTool, *stage, cue, diagnostic) ||
					!m_pBalanceTool->Add_ValtanStageEffectCue(patternId, stageId, actionId, cue, diagnostic)) return false;
				occurrenceId = cue.strOccurrenceId;
			}
			else
			{
				if (!CEffectV2Catalog::Get().Stage_AppendBossValtanStageBinding(m_strEffectAddAssetId,
					m_eEffectAddResourceKind == EFFECT_RESOURCE_KIND::V2_GROUP, patternId, stageId, actionId, cueStartMs, diagnostic)) return false;
				const auto snapshot = CEffectV2Catalog::Get().Get_Snapshot();
				if (!snapshot) { diagnostic = "Effect Append lost the staged V2 catalog."; return false; }
				for (const auto& binding : snapshot->Get_BossValtanBindings())
					if (binding.strPatternId == patternId && binding.strStageId == stageId && binding.strActionId == actionId &&
						binding.iStartMs == cueStartMs && (binding.strEffectId == m_strEffectAddAssetId || binding.strGroupId == m_strEffectAddAssetId))
					{ occurrenceId = BuildEffectV2BindingStableId(binding); break; }
				if (occurrenceId.empty()) { diagnostic = "Effect Append could not resolve the new binding."; return false; }
			}
			return m_pBalanceTool->Get_ValtanPatternDraft(patternId, candidate, diagnostic);
		}, effectStatus);
	}, status);
	if (!applied) return false;
	const auto& created = candidate.Stages.back();
	Select_Stage(candidate, created, DETAIL_OWNER::EFFECT, occurrenceId);
	m_bDetailsWindowVisible = true; m_strEffectEditIdentity.clear();
	m_iEffectV2AddStartMs = 0u; m_bEffectAddUsesStageClock = true;
	m_iEffectV2CatalogRevision = CEffectV2Catalog::Get().Get_Revision();
	m_bAuthoringDraftDirty = m_pBalanceTool->Is_ValtanDraftDirty();
	Invalidate_EffectivePatternCache(); Invalidate_TimelineCache();
	status = "Appended Effect at Pattern end (" + std::to_string(endMs) + " ms) in " + stageId + ". Set Boss or Map placement in Box Detail; Save when ready.";
	Refresh_PatternLocalPreviewAfterMutation(&candidate, status);
	return true;
}

void Client::CValtanActionWorkbench::Render_ResourcesPane(
	const VALTAN_PATTERN_VIEW* const pPattern,
	const VALTAN_STAGE_VIEW* const pStage,
	const bool_t bMutationAdmitted,
	const bool_t bPatternMutationAdmitted)
{
	m_bCompositionResourcesFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
	// 선택은 명시적인 Stage 추가 대상이며 숨겨 둔 고정 ID를 사용하지 않는다.
	// 아래 Pattern-end Append는 이 대상이나 선택 Animation에 의존하지 않는다.
	const VALTAN_PATTERN_VIEW* pResourcePattern = pPattern;
	const VALTAN_STAGE_VIEW* pResourceStage = nullptr;
	if (pPattern && pPattern->strPatternId == m_strSelectedPatternId)
	{
		const auto selected = std::find_if(pPattern->Stages.begin(), pPattern->Stages.end(),
			[this](const auto& stage) { return stage.strStageId == m_strSelectedStageId; });
		if (selected != pPattern->Stages.end()) pResourceStage = &*selected;
	}
	if (pResourcePattern && pResourceStage &&
		(m_strResourceTargetPatternId != pResourcePattern->strPatternId ||
		 m_strResourceTargetStageId != pResourceStage->strStageId))
	{
		m_strResourceTargetPatternId = pResourcePattern->strPatternId;
		m_strResourceTargetStageId = pResourceStage->strStageId;
		const auto* clip = ResolveEffectAppendClip(*pResourceStage, {}, m_strSelectedStableId,
			ResolveAppendCursorClipId(m_TimelineItems, pResourcePattern->strPatternId, pResourceStage->strStageId, m_iPlayheadMs));
		m_strResourceTargetClipOccurrenceId = clip ? clip->strClipOccurrenceId : std::string{};
		m_strEffectAddClipOccurrenceId = m_strResourceTargetClipOccurrenceId;
		m_strSoundAddClipOccurrenceId.clear();
		m_iEffectV2AddStartMs = 0u;
	}
	ImGui::TextDisabled("Append: Pattern end | Add to selected Stage: %s",
		pResourceStage ? pResourceStage->strStageId.c_str() : "select a Stage");
	const auto ResourceTabFlags = [this](const RESOURCE_DOMAIN eDomain)
	{
		return m_bResourceDomainSelectionRequested &&
			m_eRequestedResourceDomain == eDomain ?
			ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
	};
	bool_t bResourceSelectionConsumed = false;
	if (ImGui::BeginTabBar("##CompositionResourceDomainTabs"))
	{
		for (std::size_t index = 0u; index < COMPOSITION_RESOURCE_CATEGORIES.size(); ++index)
		{
			const auto domain = static_cast<RESOURCE_DOMAIN>(index);
			if (!ImGui::BeginTabItem(COMPOSITION_RESOURCE_CATEGORIES[index], nullptr, ResourceTabFlags(domain)))
				continue;
			bResourceSelectionConsumed |= m_bResourceDomainSelectionRequested && m_eRequestedResourceDomain == domain;
		if (domain == RESOURCE_DOMAIN::ANIMATION)
		{
			bResourceSelectionConsumed |= m_bResourceDomainSelectionRequested &&
				RESOURCE_DOMAIN::ANIMATION == m_eRequestedResourceDomain;
			Render_SequenceBrowser(
				pResourcePattern, pResourceStage, bPatternMutationAdmitted);
		}
		if (domain == RESOURCE_DOMAIN::EFFECT)
		{
			bResourceSelectionConsumed |= m_bResourceDomainSelectionRequested &&
				RESOURCE_DOMAIN::EFFECT == m_eRequestedResourceDomain;
			if (!m_bSemanticValtanEffectLoadAttempted)
				Reload_SemanticValtanEffects();
			if (ImGui::Button("Refresh Effect Catalog"))
				Reload_SemanticValtanEffects();
			if (!m_strEffectCatalogStatus.empty())
				ImGui::TextWrapped("%s", m_strEffectCatalogStatus.c_str());
			ImGui::SetNextItemWidth(-1.f);
			if (ImGui::InputTextWithHint(
				"##CompositionResourceEffectSearch", "Search pattern name, saved Effect, group or ID...",
				m_EffectSearch.data(), m_EffectSearch.size()))
			{
				m_bEffectFilterDirty = true;
			}
			if (ImGui::Checkbox("Show other owners", &m_bShowOtherEffectOwners))
				m_bEffectFilterDirty = true;
			ImGui::TextWrapped("Open Editor edits the reusable Effect. Append adds a separate Pattern box; it does not replace existing Effects. Rock spawns belong to Summon, hit geometry to Collider.");
			const auto IsFullRestore = [this](const EFFECT_RESOURCE_KIND kind, const std::string& id)
			{
				const auto label = m_EffectResourceLabels.find(id);
				return kind == EFFECT_RESOURCE_KIND::V1_PATTERN && label != m_EffectResourceLabels.end() && label->second.fullRestore;
			};
			const auto EffectName = [this, &IsFullRestore](const EFFECT_RESOURCE_KIND kind, const std::string& id)
			{
				const auto found = m_EffectResourceLabels.find(id);
				std::string name = found == m_EffectResourceLabels.end() ? id : found->second.displayName;
				if (IsFullRestore(kind, id)) name += " [Full Restore]";
				return name;
			};
			const std::string Query = m_EffectSearch.data();
			if (m_bEffectFilterDirty || m_strEffectFilterQuery != Query)
			{
				m_FilteredEffectAssetIndices.clear();
				m_FilteredEffectV2DocumentIndices.clear();
				m_FilteredEffectV2GroupIndices.clear();
				m_FilteredEffectAssetIndices.reserve(
					m_SemanticValtanEffectAssetIds.size());
				for (std::size_t i = 0u;
					i < m_SemanticValtanEffectAssetIds.size(); ++i)
				{
					const auto& id = m_SemanticValtanEffectAssetIds[i];
					const auto label = m_EffectResourceLabels.find(id);
					if ((m_bShowOtherEffectOwners || id.starts_with("effect.valtan.")) &&
						(ContainsInsensitive(label == m_EffectResourceLabels.end() ? id : label->second.searchText, Query) ||
						 ContainsInsensitive(EffectName(EFFECT_RESOURCE_KIND::V1_PATTERN, id), Query)))
					{
						m_FilteredEffectAssetIndices.push_back(i);
					}
				}
				for (std::size_t i = 0u; i < m_EffectV2DocumentIds.size(); ++i)
				{
					if ((m_bShowOtherEffectOwners || m_EffectV2DocumentIds[i].starts_with("boss.valtan.")) && ContainsInsensitive(m_EffectResourceLabels.at(m_EffectV2DocumentIds[i]).searchText, Query))
						m_FilteredEffectV2DocumentIndices.push_back(i);
				}
				for (std::size_t i = 0u; i < m_EffectV2GroupIds.size(); ++i)
				{
					if ((m_bShowOtherEffectOwners || m_EffectV2GroupIds[i].starts_with("boss.valtan.")) && ContainsInsensitive(m_EffectResourceLabels.at(m_EffectV2GroupIds[i]).searchText, Query))
						m_FilteredEffectV2GroupIndices.push_back(i);
				}

				m_EffectResourceRows.clear();
				for (const auto index : m_FilteredEffectAssetIndices)
					m_EffectResourceRows.push_back({ EFFECT_RESOURCE_KIND::V1_PATTERN, m_SemanticValtanEffectAssetIds[index] });
				for (const auto index : m_FilteredEffectV2DocumentIndices)
					m_EffectResourceRows.push_back({ EFFECT_RESOURCE_KIND::V2_LEAF, m_EffectV2DocumentIds[index] });
				for (const auto index : m_FilteredEffectV2GroupIndices)
					m_EffectResourceRows.push_back({ EFFECT_RESOURCE_KIND::V2_GROUP, m_EffectV2GroupIds[index] });
				std::stable_sort(m_EffectResourceRows.begin(), m_EffectResourceRows.end(), [&](const auto& left, const auto& right)
					{ return m_EffectResourceLabels.at(left.assetId).displayName < m_EffectResourceLabels.at(right.assetId).displayName; });
				m_EffectResourceTree = {};
				for (size_t index = 0u; index < m_EffectResourceRows.size(); ++index)
				{
					const auto& resource = m_EffectResourceRows[index];
					const auto& metadata = m_EffectResourceLabels.at(resource.assetId);
					if (IsFullRestore(resource.kind, resource.assetId))
					{
						// 추가 대상에 cue가 하나도 없어도 원본 clip 연결을 표시한다.
						std::set<std::string> patternNames;
						for (const auto& path : metadata.patternCategories)
							if (path.size() >= 3u && path[0] == "Patterns") patternNames.insert(path[1]);
						if (patternNames.empty()) patternNames.insert("Other restored sources");
						for (const auto& name : patternNames)
							InsertResourceTree(m_EffectResourceTree, { "Full Restore", name }, index);
					}
					else InsertEffectResourcePaths(m_EffectResourceTree,
						resource.kind == EFFECT_RESOURCE_KIND::V1_PATTERN,
						resource.kind == EFFECT_RESOURCE_KIND::V2_GROUP,
						metadata.categories, metadata.patternCategories, index);
				}
				(void)FinalizeResourceTree(m_EffectResourceTree);
				m_strEffectFilterQuery = Query;
				m_bEffectFilterDirty = false;
			}
			ImGui::SeparatorText("Selected Effect -> Append Target");
			const char_t* const pEffectKind =
				EFFECT_RESOURCE_KIND::V1_PATTERN == m_eEffectAddResourceKind ?
					"V1 Pattern" :
					(EFFECT_RESOURCE_KIND::V2_LEAF == m_eEffectAddResourceKind ?
						"V2 Leaf" : "V2 Group");
			const std::string SelectedEffectName = m_strEffectAddAssetId.empty() ?
				"Select one Effect below." : EffectName(m_eEffectAddResourceKind, m_strEffectAddAssetId);
			ImGui::TextWrapped("%s", SelectedEffectName.c_str());
			if (ImGui::IsItemHovered() && !m_strEffectAddAssetId.empty())
				ImGui::SetTooltip("%s\n%s", pEffectKind, m_strEffectAddAssetId.c_str());
			if (const auto selected = m_EffectResourceLabels.find(m_strEffectAddAssetId);
				selected != m_EffectResourceLabels.end() && !selected->second.status.empty())
				ImGui::TextWrapped("Selected source: %s", selected->second.status.c_str());
			const bool_t bResourcePreviewAdmitted = !m_strEffectAddAssetId.empty() &&
				!IsValtanServerPlaybackBusy(m_pValtanBossTool);
			ImGui::BeginDisabled(!bResourcePreviewAdmitted);
			if (ImGui::Button("Preview Selected Effect##ResourceEffect") && bResourcePreviewAdmitted)
			{
				m_PendingEffectResourcePreview = EFFECT_RESOURCE_KEY{
					m_eEffectAddResourceKind == EFFECT_RESOURCE_KIND::V1_PATTERN ? EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT :
					m_eEffectAddResourceKind == EFFECT_RESOURCE_KIND::V2_LEAF ? EFFECT_RESOURCE_OWNER_KIND::V2_LEAF : EFFECT_RESOURCE_OWNER_KIND::V2_GROUP,
					m_strEffectAddAssetId };
			}
			ImGui::EndDisabled();
			if (!m_strEffectAddAssetId.empty()) ImGui::SameLine();
			if (!m_strEffectAddAssetId.empty() && ImGui::Button("Open Editor##ResourceEffect"))
			{
				m_PendingEffectResourceOpen = EFFECT_RESOURCE_KEY{
					m_eEffectAddResourceKind == EFFECT_RESOURCE_KIND::V1_PATTERN ? EFFECT_RESOURCE_OWNER_KIND::V1_DOCUMENT :
					m_eEffectAddResourceKind == EFFECT_RESOURCE_KIND::V2_LEAF ? EFFECT_RESOURCE_OWNER_KIND::V2_LEAF : EFFECT_RESOURCE_OWNER_KIND::V2_GROUP,
					m_strEffectAddAssetId };
			}
			if (!m_strEffectAddAssetId.empty()) ImGui::SameLine();
			if (!m_strEffectAddAssetId.empty() && ImGui::Button("Copy Selected Effect Resource"))
			{
				CCompositionClipboard::Get().Write(CaptureEffectResource(m_strEffectAddAssetId,
					m_eEffectAddResourceKind == EFFECT_RESOURCE_KIND::V1_PATTERN ? "V1_EFFECT" :
					m_eEffectAddResourceKind == EFFECT_RESOURCE_KIND::V2_LEAF ? "LEAF" : "GROUP"));
				m_strStatus = "Copied the reusable Effect resource.";
			}
			const bool_t bV1Selection =
				EFFECT_RESOURCE_KIND::V1_PATTERN == m_eEffectAddResourceKind;
			ImGui::TextWrapped("Append places the Effect after the Pattern end. Existing gameplay Stages are preserved; no Stage or Animation selection is required.");
			const bool appendEndReady = pResourcePattern && m_pBalanceTool && bPatternMutationAdmitted &&
				!m_strEffectAddAssetId.empty() && (bV1Selection || bMutationAdmitted);
			ImGui::BeginDisabled(!appendEndReady);
			const bool appendEnd = ImGui::Button("Append Effect to Pattern End") && appendEndReady;
			ImGui::EndDisabled();
			if (appendEnd && Append_EffectResourceAtPatternEnd(pResourcePattern->strPatternId, m_strStatus))
			{
				m_bResourceDomainSelectionRequested = false;
				ImGui::EndTabItem(); ImGui::EndTabBar();
				return; // 이 변경으로 effective Pattern 저장소가 교체되었다.
			}
			if (ImGui::CollapsingHeader("Add to selected Stage (optional)##ResourceEffectTarget"))
			{
			if (bV1Selection)
			{
				ImGui::Checkbox("Independent Effect (Stage clock)", &m_bEffectAddUsesStageClock);
				if (m_bEffectAddUsesStageClock)
				{
					ImGui::InputScalar("Stage start (ms)##ResourceEffect", ImGuiDataType_U32, &m_iEffectV2AddStartMs);
					ImGui::TextDisabled("No Animation is required. Set Boss or Map placement in Box Detail after Append.");
				}
				const VALTAN_CLIP_OCCURRENCE_VIEW* pEffectTargetClip = nullptr;
				if (nullptr != pResourceStage)
				{
					pEffectTargetClip = ResolveEffectAppendClip(*pResourceStage,
						m_strEffectAddClipOccurrenceId, m_strSelectedStableId,
						ResolveAppendCursorClipId(m_TimelineItems, pResourcePattern->strPatternId,
							pResourceStage->strStageId, m_iPlayheadMs));
					if (nullptr != pEffectTargetClip)
						m_strEffectAddClipOccurrenceId = pEffectTargetClip->strClipOccurrenceId;
				}
				const std::string EffectClipPreview = nullptr == pEffectTargetClip ?
					"No Animation in this Stage" : pEffectTargetClip->strClipName + " | " +
						pEffectTargetClip->strClipOccurrenceId;
				if (!m_bEffectAddUsesStageClock && ImGui::BeginCombo(
						"Attach to Animation Box##ResourceEffect",
						EffectClipPreview.c_str()))
				{
					if (nullptr != pResourceStage)
					{
						for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
							pResourceStage->ClipOccurrences)
						{
							const bool_t bSelected = Clip.strClipOccurrenceId ==
								m_strEffectAddClipOccurrenceId;
							const std::string Label = Clip.strClipName + " | " +
								Clip.strClipOccurrenceId;
							if (ImGui::Selectable(Label.c_str(), bSelected))
							{
								m_strEffectAddClipOccurrenceId = Clip.strClipOccurrenceId;
								pEffectTargetClip = &Clip;
							}
						}
					}
					ImGui::EndCombo();
				}
				const bool_t bEffectAppendAdmitted =
					nullptr != pResourcePattern && nullptr != pResourceStage &&
					nullptr != m_pBalanceTool && bPatternMutationAdmitted &&
					(m_bEffectAddUsesStageClock ? m_iEffectV2AddStartMs < pResourceStage->iDurationMs :
						(nullptr != pEffectTargetClip && "WAIT" != pResourceStage->strSequenceRole)) &&
					!m_strEffectAddAssetId.empty();
				ImGui::BeginDisabled(!bEffectAppendAdmitted);
				if (ImGui::Button("Append Effect to Pattern Draft") &&
					bEffectAppendAdmitted)
				{
					VALTAN_PRODUCT_EFFECT_CUE_VIEW Cue;
					Cue.strBindingId = BuildNextCompositionEffectCueId(
						*pResourcePattern, *pResourceStage);
					Cue.strOccurrenceId = Cue.strBindingId + ".occurrence.01";
					Cue.strPatternId = pResourcePattern->strPatternId;
					Cue.strStageId = pResourceStage->strStageId;
					Cue.strActionId = pResourceStage->strActionId;
					Cue.bUsesStageClock = m_bEffectAddUsesStageClock;
					Cue.strClipOccurrenceId = Cue.bUsesStageClock ? std::string{} : pEffectTargetClip->strClipOccurrenceId;
					Cue.iStageOffsetMs = Cue.bUsesStageClock ? m_iEffectV2AddStartMs : 0u;
					Cue.strEffectAssetId = m_strEffectAddAssetId;
					Cue.strAnchorSlotId = "root";
					Cue.eFollowPolicy = EFFECT_FOLLOW_POLICY::FOLLOW;
					Cue.strFollowPolicy = "follow";
					Cue.eStopPolicy = EFFECT_STOP_POLICY::NATURAL;
					Cue.strStopPolicy = "natural";
					Cue.strRepeatPolicy = "once";
					Cue.eScalePolicy =
						VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE;
					Cue.strScalePolicy = "OWNER_RELATIVE";
					Cue.bHasExplicitScalePolicy = true;
					Cue.iSourceStartMs = Cue.bUsesStageClock ? Cue.iStageOffsetMs : pEffectTargetClip->iSourceStartMs;
					Cue.iStageDurationMs = pResourceStage->iDurationMs;
					std::string Status;
					if (!Cue.strBindingId.empty() &&
						ValidateValtanEffectInvocationClock(m_pAnimationTool, *pResourceStage, Cue, Status) &&
						m_pBalanceTool->Add_ValtanStageEffectCue(
							pResourcePattern->strPatternId,
							pResourceStage->strStageId,
							pResourceStage->strActionId, Cue, Status))
					{
						Select_Stage(*pResourcePattern, *pResourceStage, DETAIL_OWNER::EFFECT, Cue.strOccurrenceId);
						m_eDetailOwner = DETAIL_OWNER::EFFECT;
						m_strSelectedStableId = Cue.strOccurrenceId;
						m_strEffectEditIdentity.clear();
						m_bDetailsWindowVisible = true;
						Invalidate_TimelineCache();
						Refresh_PatternLocalPreviewAfterMutation(pResourcePattern, Status);
					}
					m_strStatus = Cue.strBindingId.empty() ?
						"Effect append rejected: no free stable invocation ID is available." :
						std::move(Status);
				}
				ImGui::EndDisabled();
				if (!bEffectAppendAdmitted)
				{
					if (nullptr == pResourcePattern || nullptr == pResourceStage)
						ImGui::TextWrapped("Select a Stage for this optional add, or use Append Effect to Pattern End above.");
					else if (!bPatternMutationAdmitted || nullptr == m_pBalanceTool)
						ImGui::TextWrapped("Append needs the admitted editable Pattern draft; finish Save/publish or reload the current source revision.");
					else if (m_bEffectAddUsesStageClock && m_iEffectV2AddStartMs >= pResourceStage->iDurationMs)
						ImGui::TextWrapped("Effect start must be inside the target Stage; extend its duration or choose an earlier start.");
					else if (!m_bEffectAddUsesStageClock && "WAIT" == pResourceStage->strSequenceRole)
						ImGui::TextWrapped("V1 Effects attach to Animation boxes; select a Stage with an Animation.");
					else if (!m_bEffectAddUsesStageClock && nullptr == pEffectTargetClip)
						ImGui::TextWrapped("Append needs an Animation in this Stage. Add an Animation or use a Stage that already has one.");
					else if (m_strEffectAddAssetId.empty())
						ImGui::TextWrapped("Select a saved V1 Effect below.");
				}
			}
			else
			{
				const uint32_t iStepMs = 1u;
				const uint32_t iFastStepMs = 100u;
				ImGui::BeginDisabled(nullptr == pResourceStage);
				if (ImGui::InputScalar(
						"Stage-local start (ms)##ResourceEffectV2",
						ImGuiDataType_U32, &m_iEffectV2AddStartMs,
						&iStepMs, &iFastStepMs, "%u") && nullptr != pResourceStage)
				{
					m_iEffectV2AddStartMs = (std::min)(
						m_iEffectV2AddStartMs, pResourceStage->iDurationMs > 0u ? pResourceStage->iDurationMs - 1u : 0u);
				}
				ImGui::EndDisabled();
				const bool_t bV2AppendAdmitted =
					nullptr != pResourcePattern && nullptr != pResourceStage &&
					bMutationAdmitted && m_iEffectV2AddStartMs < pResourceStage->iDurationMs &&
					!m_strEffectAddAssetId.empty();
				ImGui::BeginDisabled(!bV2AppendAdmitted);
				if (ImGui::Button("Append V2 Stage Binding") &&
					bV2AppendAdmitted)
				{
					std::string Status;
					if (CEffectV2Catalog::Get().Stage_AppendBossValtanStageBinding(
							m_strEffectAddAssetId,
							EFFECT_RESOURCE_KIND::V2_GROUP ==
								m_eEffectAddResourceKind,
							pResourcePattern->strPatternId,
							pResourceStage->strStageId,
							pResourceStage->strActionId,
							m_iEffectV2AddStartMs, Status))
					{
						m_iEffectV2CatalogRevision =
							CEffectV2Catalog::Get().Get_Revision();
						Refresh_EffectV2LocalPreviewAfterMutation(
							pResourcePattern, Status);
					}
					m_strEffectCatalogStatus = Status;
					m_strStatus = std::move(Status);
				}
				ImGui::EndDisabled();
				if (!bV2AppendAdmitted)
				{
					if (nullptr == pResourcePattern || nullptr == pResourceStage)
						ImGui::TextWrapped("Select a Stage for this optional add, or use Append Effect to Pattern End above.");
					else if (!bMutationAdmitted)
						ImGui::TextWrapped("Append needs the admitted editable Pattern draft; finish Save/publish or reload the current source revision.");
					else if (m_iEffectV2AddStartMs >= pResourceStage->iDurationMs)
						ImGui::TextWrapped("The V2 start must be earlier than the selected Stage end.");
					else if (m_strEffectAddAssetId.empty())
						ImGui::TextWrapped("Select a saved V2 Group or Leaf below.");
				}
				ImGui::TextDisabled(
					"V2 changes stay in this Composition draft. The Sequencer Save commits Pattern, Sound and V2 as one rollback-safe transaction.");
			}

			}

			ImGui::SeparatorText("Effect Resources");
			ImGui::TextWrapped("Full Restore is grouped by source Pattern. Select an Effect, Preview it, then Append it to the Pattern end.");
			const auto renderEffect = [this, &EffectName, &IsFullRestore](size_t index)
			{
				const auto& resource = m_EffectResourceRows[index];
				const auto& metadata = m_EffectResourceLabels.at(resource.assetId);
				const char* kind = resource.kind == EFFECT_RESOURCE_KIND::V1_PATTERN ? "V1 Effect" :
					resource.kind == EFFECT_RESOURCE_KIND::V2_GROUP ? "V2 Group" : "V2 Leaf";
				const std::string name = EffectName(resource.kind, resource.assetId);
				const std::string label = name + (IsFullRestore(resource.kind, resource.assetId) ? std::string{} : " [" + std::string(kind) + "]") + "##" + resource.assetId;
				const bool selected = m_eEffectAddResourceKind == resource.kind && m_strEffectAddAssetId == resource.assetId;
				if (ImGui::Selectable(label.c_str(), selected))
				{
					m_eEffectAddResourceKind = resource.kind;
					m_strEffectAddAssetId = resource.assetId;
				}
				if (ImGui::IsItemHovered())
				{
					std::string path;
					for (const auto& category : metadata.categories) path += (path.empty() ? "" : " / ") + category;
					ImGui::SetTooltip("%s\n%s\n%s\n%s", name.c_str(), resource.assetId.c_str(), path.c_str(), metadata.status.c_str());
				}
				Offer_CompositionResourceDrag(name.c_str(), [resource]()
					{ return CaptureEffectResource(resource.assetId, resource.kind == EFFECT_RESOURCE_KIND::V1_PATTERN ? "V1_EFFECT" :
						resource.kind == EFFECT_RESOURCE_KIND::V2_GROUP ? "GROUP" : "LEAF"); });
			};
			ImGui::TextDisabled("%zu matching Effects", m_EffectResourceRows.size());
			const auto renderEffectTree = [&](auto&& self, const COMPOSITION_RESOURCE_TREE_NODE& node, const bool restoreBranch) -> void
			{
				for (const auto& child : node.Children)
				{
					const bool restore = restoreBranch || child.strSegment == "Full Restore";
					ImGui::PushID(child.strStablePath.c_str());
					if (!Query.empty()) ImGui::SetNextItemOpen(true, ImGuiCond_Always);
					else if (restore) ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
					if (ImGui::TreeNodeEx("##EffectResourceCategory", ImGuiTreeNodeFlags_SpanAvailWidth,
						"%s (%zu)", child.strSegment.c_str(), child.iRecursiveLeafCount))
					{
						self(self, child, restore);
						ImGui::TreePop();
					}
					ImGui::PopID();
				}
				for (const auto index : node.LeafIndices) renderEffect(index);
			};
			// Preview와 Append는 별도로 스크롤하는 library 위에 둔다.
			// 부모 창을 늘리지 않고 pane의 남은 높이를 사용한다.
			const float effectTreeHeight = (std::max)(1.f, (std::min)(
				ImGui::GetTextLineHeightWithSpacing() * 32.f,
				ImGui::GetContentRegionAvail().y - ImGui::GetStyle().ItemSpacing.y));
			if (ImGui::BeginChild("##CompositionResourceEffectTree",
				ImVec2(0.f, effectTreeHeight), ImGuiChildFlags_Borders,
				ImGuiWindowFlags_AlwaysVerticalScrollbar))
			{
				renderEffectTree(renderEffectTree, m_EffectResourceTree, false);
				if (m_EffectResourceRows.empty()) ImGui::TextDisabled("No matching saved Effects.");
			}
			ImGui::EndChild();
		}
		if (domain == RESOURCE_DOMAIN::SOUND)
		{
			bResourceSelectionConsumed |= m_bResourceDomainSelectionRequested &&
				RESOURCE_DOMAIN::SOUND == m_eRequestedResourceDomain;
			if (m_strSoundAddEvent.empty() && !m_PatternSoundEvents.empty())
				m_strSoundAddEvent = m_PatternSoundEvents.front();
			ImGui::SetNextItemWidth(-1.f);
			if (ImGui::InputTextWithHint(
					"##CompositionResourceSoundSearch", "Search Sound event...",
					m_SoundSearch.data(), m_SoundSearch.size()))
			{
				m_bSoundFilterDirty = true;
			}
			const std::string SoundQuery = m_SoundSearch.data();
			if (m_bSoundFilterDirty || m_strSoundFilterQuery != SoundQuery)
			{
				m_FilteredSoundEventIndices.clear();
				m_SoundResourceTree = {};
				for (std::size_t i = 0u; i < m_PatternSoundEvents.size(); ++i)
				{
					if (!ContainsInsensitive(m_PatternSoundEvents[i], SoundQuery))
						continue;
					m_FilteredSoundEventIndices.push_back(i);
					std::vector<std::string> Segments =
						SplitResourcePath(m_PatternSoundEvents[i]);
					if (!Segments.empty())
						Segments.pop_back();
					InsertResourceTree(m_SoundResourceTree, Segments, i);
				}
				(void)FinalizeResourceTree(m_SoundResourceTree);
				m_strSoundFilterQuery = SoundQuery;
				m_bSoundFilterDirty = false;
			}
			const VALTAN_CLIP_OCCURRENCE_VIEW* pSoundTargetClip = nullptr;
			if (nullptr != pResourceStage)
			{
				pSoundTargetClip = ResolveEffectAppendClip(*pResourceStage,
					m_strSoundAddClipOccurrenceId, m_strSelectedStableId,
					ResolveAppendCursorClipId(m_TimelineItems, pResourcePattern->strPatternId,
						pResourceStage->strStageId, m_iPlayheadMs));
				if (nullptr != pSoundTargetClip &&
					m_strSoundAddClipOccurrenceId != pSoundTargetClip->strClipOccurrenceId)
				{
					m_strSoundAddClipOccurrenceId = pSoundTargetClip->strClipOccurrenceId;
					m_iSoundAddStartMs = pSoundTargetClip->iSourceStartMs;
					m_eSoundAddRepeatPolicy = VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
				}
			}
			ImGui::SeparatorText("Selected Sound -> Append Target");
			ImGui::TextWrapped("%s",
				m_strSoundAddEvent.empty() ?
					"Select one Sound event below." :
					m_strSoundAddEvent.c_str());
			const char_t* const pSoundClipPreview =
				nullptr == pSoundTargetClip ?
					"Select Animation box" :
					pSoundTargetClip->strClipName.c_str();
			if (ImGui::BeginCombo(
					"Attach to Animation Box##ResourceSound",
					pSoundClipPreview))
			{
				if (nullptr != pResourceStage)
				{
					for (const VALTAN_CLIP_OCCURRENCE_VIEW& Clip :
						pResourceStage->ClipOccurrences)
					{
						const bool_t bSelected = Clip.strClipOccurrenceId ==
							m_strSoundAddClipOccurrenceId;
						const std::string Label = Clip.strClipName + " | " +
							Clip.strClipOccurrenceId;
						if (ImGui::Selectable(Label.c_str(), bSelected))
						{
							m_strSoundAddClipOccurrenceId =
								Clip.strClipOccurrenceId;
							m_iSoundAddStartMs = Clip.iSourceStartMs;
							m_eSoundAddRepeatPolicy =
								VALTAN_PATTERN_SOUND_REPEAT_POLICY::ONCE;
						}
					}
				}
				ImGui::EndCombo();
			}
			/* 여기서는 보이는 행의 버튼을 그리며 native 모델 구간 검증은 Add 명령을 누를 때 수행한다.
			   그 검증은 모든 animation을 순회할 수 있으므로 매 ImGui 프레임의 버튼 활성 검사에서 반복하지 않는다. */
			const bool_t bSoundTargetReady =
				nullptr != pResourceStage && nullptr != pSoundTargetClip &&
				nullptr != m_pAnimationTool;
			ImGui::BeginDisabled(!bSoundTargetReady);
			const uint32_t iStepMs = 1u;
			const uint32_t iFastStepMs = 100u;
			if (ImGui::InputScalar(
				"Source start (ms)##ResourceSound", ImGuiDataType_U32,
				&m_iSoundAddStartMs, &iStepMs, &iFastStepMs, "%u"))
			{
				m_iSoundAddStartMs = (std::min)(
					m_iSoundAddStartMs, uint32_t{ 600000u });
			}
			ImGui::EndDisabled();
			const bool_t bSoundAppendAdmitted =
				nullptr != pResourcePattern && nullptr != pResourceStage &&
				nullptr != pSoundTargetClip && nullptr != m_pAnimationTool &&
				bMutationAdmitted && bSoundTargetReady &&
				!m_strSoundAddEvent.empty();
			ImGui::BeginDisabled(!bSoundAppendAdmitted);
			if (ImGui::Button("Append Selected Sound to Stage") &&
				bSoundAppendAdmitted)
			{
				VALTAN_PATTERN_SOUND_CUE_ROW_ID Created;
				std::string Status;
				if (m_pAnimationTool->Add_ValtanCompositionPatternSound(
						*pResourcePattern, *pResourceStage,
						pSoundTargetClip->strClipOccurrenceId,
						m_strSoundAddEvent, m_iSoundAddStartMs,
						m_eSoundAddRepeatPolicy, Created, Status))
				{
					m_eDetailOwner = DETAIL_OWNER::SOUND;
					m_strSelectedStableId = Created.strOccurrenceId;
					m_bDetailsWindowVisible = true;
					Invalidate_TimelineCache();
				}
				m_strSoundStatus = Status;
				m_strStatus = std::move(Status);
			}
			ImGui::EndDisabled();
			ImGui::SeparatorText("Sound Event Tree");
			RenderResourceTree(
				m_SoundResourceTree,
				[this](const std::size_t i)
				{
					const std::string& Event = m_PatternSoundEvents[i];
					const bool_t bSelected = Event == m_strSoundAddEvent;
					if (ImGui::Selectable(Event.c_str(), bSelected))
					{
						m_strSoundAddEvent = Event;
						m_eDetailOwner = DETAIL_OWNER::SOUND;
						m_bDetailsWindowVisible = true;
					}
				});
		}
		if (domain == RESOURCE_DOMAIN::WORLD)
		{
			bResourceSelectionConsumed |= m_bResourceDomainSelectionRequested &&
				RESOURCE_DOMAIN::WORLD == m_eRequestedResourceDomain;
			Render_SupplementalResources(domain, pResourcePattern, pResourceStage, bPatternMutationAdmitted);
		}
		if (domain == RESOURCE_DOMAIN::CAMERA)
		{
			bResourceSelectionConsumed |= m_bResourceDomainSelectionRequested &&
				RESOURCE_DOMAIN::CAMERA == m_eRequestedResourceDomain;
			Render_SupplementalResources(domain, pResourcePattern, pResourceStage, bPatternMutationAdmitted);
		}
			if (domain == RESOURCE_DOMAIN::LOGIC || domain == RESOURCE_DOMAIN::SUMMON ||
				domain == RESOURCE_DOMAIN::SCENE_PROFILE || domain == RESOURCE_DOMAIN::COLLIDER ||
				domain == RESOURCE_DOMAIN::LIGHT || domain == RESOURCE_DOMAIN::PATTERN)
				Render_SupplementalResources(domain, pResourcePattern, pResourceStage, bPatternMutationAdmitted);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	if (bResourceSelectionConsumed)
		m_bResourceDomainSelectionRequested = false;
}

void Client::CValtanActionWorkbench::On_WorkbenchDeactivated()
{
	m_PendingLocalPreview.reset();
    m_BoneEditor.Stop();
	m_bPreviewOwnerActive = false;
	m_bPreviewOwnerClaimRequested = false;
	// 공용 Sequence 창은 대상 보스가 바뀌어도 열려 있을 수 있다.
	// 조작부만 숨기지 말고 이 세션이 실제로 시작한 재생을 여기서 중지한다.
	if (nullptr != m_pAnimationTool)
	{
		const auto Preview = m_pAnimationTool->Get_ValtanCompositionPreviewState();
		if (Preview.bPlaying || Preview.bSourceSequencePlaying)
		{
			std::string Status;
			m_pAnimationTool->Stop_ValtanCompositionPattern(Status);
			m_strStatus = std::move(Status);
		}
	}
}

struct Client::CValtanActionWorkbench::HISTORY_SCOPE final
{
    CValtanActionWorkbench& owner;
    bool standalone;
    explicit HISTORY_SCOPE(CValtanActionWorkbench& value) : owner(value),
        standalone(!value.m_bWorkbenchFrameActive && !value.m_FrameHistoryBefore && !value.m_EditHistory.Is_Pending())
    { if (standalone) owner.Begin_EditHistory(); }
    ~HISTORY_SCOPE() { if (standalone) owner.Finish_EditHistory(true); }
};

struct Client::CValtanActionWorkbench::EDIT_HISTORY_VALUES final
{
    std::shared_ptr<const CBalanceTool::VALTAN_EDIT_SNAPSHOT> balance;
    std::optional<VALTAN_PATTERN_SOUND_CUE_DOCUMENT> sounds;
    std::shared_ptr<const EFFECT_V2_CATALOG_SNAPSHOT> effects;
    VALTAN_PATTERN_SHAKE_CUE_DOCUMENT shakes;
    VALTAN_COMBAT_OBJECT_SOUND_CUE_DOCUMENT objectSounds;
};

struct Client::CValtanActionWorkbench::EDIT_HISTORY_STATE final
{
    std::shared_ptr<const EDIT_HISTORY_VALUES> values;
    std::vector<TIMELINE_SELECTION> selection;
    std::string pattern, stage, stable;
    DETAIL_OWNER owner = DETAIL_OWNER::PATTERN;
    uint32_t cursor = 0u;
};

Client::CValtanActionWorkbench::HISTORY_STATE Client::CValtanActionWorkbench::Capture_EditHistory()
{
    if (!m_pBalanceTool || !m_pAnimationTool || !Is_FullyAdmitted()) return {};
    const std::array<std::uint64_t, 5> generations{
        m_pBalanceTool->Get_ValtanDraftGeneration(), m_pAnimationTool->Get_ValtanCompositionPatternSoundDraftGeneration(),
        CEffectV2Catalog::Get().Get_Revision(), m_PatternShakes.iDraftGeneration, m_CombatObjectSounds.iDraftGeneration};
    if (!m_HistoryValues || generations != m_HistoryGenerations)
    {
        auto values = std::make_shared<EDIT_HISTORY_VALUES>();
        values->balance = m_pBalanceTool->Capture_ValtanEditHistory();
        bool dirty = false; std::string status;
        if (const auto* sounds = m_pAnimationTool->Get_ValtanCompositionPatternSoundDraft(dirty, status)) values->sounds = *sounds;
        values->effects = CEffectV2Catalog::Get().Get_Snapshot();
        values->shakes = m_PatternShakes; values->objectSounds = m_CombatObjectSounds;
        m_HistoryValues = std::move(values); m_HistoryGenerations = generations;
    }
    auto state = std::make_shared<EDIT_HISTORY_STATE>();
    state->values = m_HistoryValues; state->selection = m_TimelineSelection;
    state->pattern = m_strSelectedPatternId; state->stage = m_strSelectedStageId;
    state->stable = m_strSelectedStableId; state->owner = m_eDetailOwner; state->cursor = m_iPlayheadMs;
    return state;
}

void Client::CValtanActionWorkbench::Begin_EditHistory()
{
    if (!m_EditHistory.Is_Pending() && !m_FrameHistoryBefore)
    {
        const auto previous = m_HistoryValues;
        m_FrameHistoryBefore = Capture_EditHistory();
        // Workbench가 비활성인 동안 다른 소유 도구가 편집할 수 있다.
        // 그 변경을 오래된 local 이력으로 조용히 덮어쓰지 않도록 세대를 대조한다.
        if (previous && m_FrameHistoryBefore && previous != m_FrameHistoryBefore->values) m_EditHistory.Clear();
    }
}

void Client::CValtanActionWorkbench::Finish_EditHistory(const bool force)
{
    const auto after = Capture_EditHistory();
    if (!after) { m_FrameHistoryBefore.reset(); return; }
    if (m_FrameHistoryBefore && m_FrameHistoryBefore->values != after->values)
        m_EditHistory.Begin(m_FrameHistoryBefore);
    m_FrameHistoryBefore.reset();
    if (!m_EditHistory.Is_Pending()) return;
    if (!force && ImGui::GetCurrentContext() && (ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsAnyItemActive())) return;
    m_EditHistory.Commit(after, [](const auto& before, const auto& current) { return before->values == current->values; });
}

void Client::CValtanActionWorkbench::Clear_EditHistory()
{
    m_EditHistory.Clear(); m_FrameHistoryBefore.reset(); m_HistoryValues.reset(); m_PendingHistoryDirection = 0;
}

bool Client::CValtanActionWorkbench::Restore_EditHistory(const EDIT_HISTORY_STATE& state, std::string& status)
{
    if (!state.values || !state.values->balance || !m_pBalanceTool || !m_pAnimationTool || !Is_FullyAdmitted() ||
        m_iPendingSaveJobId || m_pBalanceTool->Is_ValtanSaveJobBlockingAuthoring() ||
        m_pBalanceTool->Is_ServerRuntimeSetPublishRunning() || m_pAnimationTool->Is_ValtanCompositionPatternTransactionActive())
    { status = "Finish the active Save, Publish or Pattern creation before Undo / Redo."; return false; }
    const auto& values = *state.values;
    // 보조 소유 도구는 현재 source 기준본 위에 stage한다.
    // 이력 항목에는 값만 있으므로 저장된 디스크 revision을 과거로 되돌릴 수 없다.
    auto shakes = values.shakes; auto objectSounds = values.objectSounds;
    shakes.strSourceBytes = m_PatternShakes.strSourceBytes; shakes.iDraftGeneration = m_PatternShakes.iDraftGeneration + 1u;
    objectSounds.strSourceBytes = m_CombatObjectSounds.strSourceBytes; objectSounds.iDraftGeneration = m_CombatObjectSounds.iDraftGeneration + 1u;
    std::string baseline, candidate; uint64_t generation = 0u;
    if (m_bPatternShakesReady)
    {
        shakes.bDraftDirty = true;
        if (!CValtanPatternShakeCueDocument::Prepare_Save(shakes, baseline, candidate, generation, status)) return false;
        shakes.bDraftDirty = candidate != baseline;
    }
    if (m_bCombatObjectSoundsReady)
    {
        objectSounds.bDraftDirty = true;
        if (!CValtanCombatObjectSoundCueDocument::Prepare_Save(objectSounds, baseline, candidate, generation, status)) return false;
        objectSounds.bDraftDirty = candidate != baseline;
    }
    const auto restoreEffects = [&](std::string& diagnostic) {
        return !values.effects || CEffectV2Catalog::Get().Restore_BossValtanBindingHistory(
            values.effects->Get_BossValtanBindings(), diagnostic);
    };
    const auto restoreSoundAndEffects = [&](std::string& diagnostic) {
        if (!values.sounds) return restoreEffects(diagnostic);
        return m_pAnimationTool->Apply_ValtanCompositionPatternSoundDraftTransaction([&](std::string& message) {
            return m_pAnimationTool->Restore_ValtanCompositionSoundHistory(*values.sounds, message) && restoreEffects(message);
        }, diagnostic);
    };
    if (!m_pBalanceTool->Apply_ValtanCompositionDraftTransaction([&](std::string& diagnostic) {
        return m_pBalanceTool->Restore_ValtanEditHistory(*values.balance, diagnostic) && restoreSoundAndEffects(diagnostic);
    }, status)) return false;
    m_PendingLocalPreview.reset(); std::string stopStatus;
    m_pAnimationTool->Stop_ValtanCompositionPattern(stopStatus);
    m_PatternShakes = std::move(shakes); m_CombatObjectSounds = std::move(objectSounds);
    m_TimelineSelection = state.selection; m_strSelectedPatternId = state.pattern;
    m_strSelectedStageId = state.stage; m_strSelectedStableId = state.stable; m_eDetailOwner = state.owner; m_iPlayheadMs = state.cursor;
    m_bTimelineMoveActive = false; m_bTimelineTrimActive = false; m_bTimelineMarqueeActive = false;
    m_BossPatternOutcomeOverrides.clear(); ++m_iBossPatternRouteGeneration;
    Reset_EffectCueEditor(); Invalidate_EffectivePatternCache(); Invalidate_TimelineCache();
    m_bAuthoringDraftDirty = m_pBalanceTool->Is_ValtanDraftDirty();
    m_HistoryValues = state.values;
    m_HistoryGenerations = {m_pBalanceTool->Get_ValtanDraftGeneration(), m_pAnimationTool->Get_ValtanCompositionPatternSoundDraftGeneration(),
        CEffectV2Catalog::Get().Get_Revision(), m_PatternShakes.iDraftGeneration, m_CombatObjectSounds.iDraftGeneration};
    m_FrameHistoryBefore.reset(); status = "Restored draft and selection. Save commits this state.";
    return true;
}

bool Client::CValtanActionWorkbench::Undo_Edit(std::string& status)
{
    Finish_EditHistory(true);
    const bool changed = m_EditHistory.Undo([&](const auto& state) { return Restore_EditHistory(*state, status); });
    m_strStatus = status; return changed;
}

bool Client::CValtanActionWorkbench::Redo_Edit(std::string& status)
{
    Finish_EditHistory(true);
    const bool changed = m_EditHistory.Redo([&](const auto& state) { return Restore_EditHistory(*state, status); });
    m_strStatus = status; return changed;
}

void Client::CValtanActionWorkbench::Render_EditHistoryButtons(const char* id)
{
    m_bHistoryWindowFocused = m_bHistoryWindowFocused || ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
    const bool busy = !Is_FullyAdmitted() || !m_pBalanceTool || m_iPendingSaveJobId ||
        m_pBalanceTool->Is_ValtanSaveJobBlockingAuthoring() || m_pBalanceTool->Is_ServerRuntimeSetPublishRunning() ||
        m_bTimelineMoveActive || m_bTimelineTrimActive;
    ImGui::PushID(id);
    ImGui::BeginDisabled(busy || m_EditHistory.Count_Undo() == 0u);
    if (ImGui::Button("Undo")) m_PendingHistoryDirection = -1;
    ImGui::EndDisabled(); ImGui::SameLine();
    ImGui::BeginDisabled(busy || m_EditHistory.Count_Redo() == 0u);
    if (ImGui::Button("Redo")) m_PendingHistoryDirection = 1;
    ImGui::EndDisabled(); ImGui::PopID();
    if (!busy && m_bHistoryWindowFocused && !ImGui::GetIO().WantTextInput && !ImGui::IsAnyItemActive() && ImGui::GetIO().KeyCtrl)
    {
        if (ImGui::IsKeyPressed(ImGuiKey_Z, false)) m_PendingHistoryDirection = ImGui::GetIO().KeyShift ? 1 : -1;
        else if (ImGui::IsKeyPressed(ImGuiKey_Y, false)) m_PendingHistoryDirection = 1;
    }
}

void Client::CValtanActionWorkbench::Process_EditHistoryRequest()
{
    Finish_EditHistory();
    const int direction = std::exchange(m_PendingHistoryDirection, 0);
    if (direction < 0) (void)Undo_Edit(m_strStatus);
    else if (direction > 0) (void)Redo_Edit(m_strStatus);
}

// 공용 Composition shell이 pane를 그리기 전에 부른다. 로드·저장 완료와 owner revision을 확인한 뒤 표시할 Pattern 하나를 정한다.
// BalanceTool의 mutable draft를 그대로 여러 창에 넘기지 않고 generation이 붙은 값 복사본을 선택한다.
// 같은 프레임의 뒤쪽 편집으로 draft가 바뀌어도 먼저 얻은 view의 generation을 바꿔 적지 않는다.
void Client::CValtanActionWorkbench::Begin_WorkbenchFrame()
{
	if (m_bWorkbenchFrameActive)
		return;
	m_bCompositionResourcesFocused = false;
    if (const auto previewBoss = CAnimationTargetService::Resolve_Boss(); previewBoss &&
        CAnimationTargetService::Resolve_AssetName() == "Valtan")
    {
        m_BoneEditor.Select("Valtan", previewBoss->Get_BodyModel());
        if (m_bPreviewOwnerActive) m_BoneEditor.Update(ImGui::GetIO().DeltaTime);
        else m_BoneEditor.Stop();
    }
    else m_BoneEditor.Stop();
	if (!m_bWorkbenchBossLoadAttempted && nullptr != m_pValtanBossTool)
	{
		m_bWorkbenchBossLoadAttempted = true;
		std::string status;
		(void)m_pValtanBossTool->Reload_CanonicalGraph(status);
	}
	Update_SaveState();
	m_bApplyResetLayoutThisFrame = m_bResetLayoutRequested;
	m_bResetLayoutRequested = false;
	if (!m_bLoadAttempted)
		(void)Reload_Canonical();
	if (m_bCanonicalReloadRetryPending &&
		ImGui::GetTime() >= m_dNextCanonicalReloadRetrySeconds)
	{
		(void)Reload_Canonical();
	}
	if (nullptr != m_pAnimationTool)
	{
		std::string CreatedPatternId;
		if (m_pAnimationTool->Consume_ValtanCompositionPatternCreated(
				CreatedPatternId))
		{
			m_bPatternsWindowVisible = true;
			m_iRequestedPatternTab = 0;
			if (Reload_Canonical())
			{
				const auto FindCreated = [&CreatedPatternId](
					const std::vector<VALTAN_PATTERN_VIEW>& Patterns)
					-> const VALTAN_PATTERN_VIEW*
				{
					const auto Found = std::find_if(
						Patterns.begin(), Patterns.end(),
						[&CreatedPatternId](const VALTAN_PATTERN_VIEW& Pattern)
						{ return Pattern.strPatternId == CreatedPatternId; });
					return Found == Patterns.end() ? nullptr : &*Found;
				};
				const VALTAN_PATTERN_VIEW* pCreated =
					FindCreated(m_CanonicalView.Gimmicks);
				if (nullptr == pCreated)
					pCreated = FindCreated(m_CanonicalView.Rotation);
				if (nullptr != pCreated)
				{
					Select_Pattern(*pCreated);
					m_PatternSearch.fill(0);
					m_bSequencerWindowVisible = true;
					m_bResourcesWindowFocusRequested = true;
					m_strStatus =
						"Created, saved, reloaded and selected Pattern: " +
						CreatedPatternId + ".";
				}
				else
				{
					m_eAdmission = VALTAN_VIEW_ADMISSION::STALE_PRESERVED;
					m_strStatus =
						"The saved Pattern was not found after reload: " +
						CreatedPatternId + ".";
				}
			}
		}
	}
	Normalize_Selection();
	const VALTAN_PATTERN_VIEW* const pSavedPattern = Find_SelectedPattern();
	std::string AuthoringRevision;
	std::string CanonicalSourceRevision;
	std::string AuthoringStatus;
	bool_t bAuthoringDirty = false;
	const bool_t bCanonicalSourceJoined = nullptr != m_pBalanceTool &&
		m_pBalanceTool->Get_ValtanCanonicalSourceRevision(
			CanonicalSourceRevision, AuthoringStatus) &&
		CanonicalSourceRevision == m_strPinnedCanonicalSourceRevision;
	const bool_t bAuthoringJoined = bCanonicalSourceJoined &&
		m_pBalanceTool->Get_ValtanAuthoringState(
			AuthoringRevision, bAuthoringDirty, AuthoringStatus) &&
		AuthoringRevision == m_strPinnedAuthoringSourceRevision;
	if (VALTAN_VIEW_ADMISSION::ADMITTED == m_eAdmission && !bCanonicalSourceJoined)
	{
		m_eAdmission = VALTAN_VIEW_ADMISSION::STALE_PRESERVED;
		m_bConfirmDiscardPatternSoundDraft = false;
		m_strStatus =
			"The Balance draft moved away from the Workbench canonical source pin; the previous composition is now read-only until Reload Canonical succeeds. " +
			AuthoringStatus;
	}
	m_bAuthoringDraftDirty = VALTAN_VIEW_ADMISSION::ADMITTED == m_eAdmission &&
		bAuthoringJoined && bAuthoringDirty;
	bool_t bEffectivePatternReady = false;
	if (VALTAN_VIEW_ADMISSION::ADMITTED == m_eAdmission && nullptr != pSavedPattern &&
		pSavedPattern->bAuthoringMasterManaged && bAuthoringJoined)
	{
		const std::uint64_t iDraftGeneration =
			m_pBalanceTool->Get_ValtanDraftGeneration();
		const bool_t bCacheMatches = m_bEffectivePatternCacheReady &&
			m_strEffectivePatternCachePatternId == pSavedPattern->strPatternId &&
			m_strEffectivePatternCacheCanonicalRevision ==
				m_strPinnedCanonicalSourceRevision &&
			m_iEffectivePatternCacheDraftGeneration == iDraftGeneration;
		if (!bCacheMatches)
		{
			VALTAN_PATTERN_VIEW StagedEffectivePattern;
			if (m_pBalanceTool->Get_ValtanPatternDraft(
					pSavedPattern->strPatternId,
					StagedEffectivePattern, AuthoringStatus))
			{
				m_EffectivePatternCache = std::move(StagedEffectivePattern);
				m_strEffectivePatternCachePatternId =
					pSavedPattern->strPatternId;
				m_strEffectivePatternCacheCanonicalRevision =
					m_strPinnedCanonicalSourceRevision;
				m_iEffectivePatternCacheDraftGeneration = iDraftGeneration;
				m_bEffectivePatternCacheReady = true;
			}
			else
			{
				Invalidate_EffectivePatternCache();
			}
		}
		bEffectivePatternReady = m_bEffectivePatternCacheReady;
	}
	if (!bEffectivePatternReady && m_bCompositionResourceDraftReady &&
		m_bEffectivePatternCacheReady && nullptr != m_pBalanceTool &&
		m_strEffectivePatternCachePatternId == m_strSelectedPatternId &&
		m_iEffectivePatternCacheDraftGeneration == m_pBalanceTool->Get_ValtanDraftGeneration())
	{
		bEffectivePatternReady = true;
		m_bAuthoringDraftDirty = m_pBalanceTool->Is_ValtanDraftDirty();
	}
	const VALTAN_PATTERN_VIEW* const pPattern = bEffectivePatternReady ?
		&m_EffectivePatternCache : pSavedPattern;
	/* 이 generation은 위에서 선택한 불변 pPattern에 대응한다.
	   같은 프레임의 다음 Sequencer widget이 live Balance draft를 바꾸더라도
	   이전 Boss Pattern 투영을 새 generation의 결과로 표시하면 안 된다. */
	const std::uint64_t iPatternViewDraftGeneration = bEffectivePatternReady ?
		m_iEffectivePatternCacheDraftGeneration : 0u;
	if (nullptr != pPattern)
	{
		if (pPattern->Stages.empty())
		{
			m_strSelectedStageId.clear();
		}
		else if (nullptr == Find_SelectedStage(pPattern))
		{
			m_strSelectedStageId = pPattern->Stages.front().strStageId;
			if (DETAIL_OWNER::PATTERN != m_eDetailOwner)
			{
				m_eDetailOwner = DETAIL_OWNER::GAMEPLAY_STAGE;
				m_strSelectedStableId = m_strSelectedStageId;
			}
		}
	}
	Ensure_TimelineCache(pPattern);
	const VALTAN_STAGE_VIEW* const pStage = Find_SelectedStage(pPattern);
	/* local Arena Clone 재생 조작은 불변 값 복사본을 소비하며 canonical 파일이나 Server 상태를 바꾸지 않는다.
	   source 소유 도구 재로드로 STALE_PRESERVED가 되어도 마지막으로 승인된 Product 복사본의 local preview는 유지한다. */
	const bool_t bLocalPreviewAdmitted = nullptr != pPattern &&
		Can_DisplayValtanView(m_eAdmission);
	const bool_t bMutationAdmitted = nullptr != pPattern &&
		pPattern->bAuthoringMasterManaged &&
		Can_MutateValtanView(m_eAdmission) && bEffectivePatternReady &&
		(nullptr == m_pAnimationTool || !m_pAnimationTool->Is_ValtanCompositionPatternTransactionActive()) &&
		(nullptr == m_pBalanceTool ||
		 (!m_pBalanceTool->Is_ValtanSaveJobBlockingAuthoring() &&
		  !m_pBalanceTool->Is_ServerRuntimeSetPublishRunning()));
	std::string SoundDependencyStatus;
	m_bPatternSoundDependencyDirty = false;
	if (VALTAN_VIEW_ADMISSION::ADMITTED == m_eAdmission)
	{
		m_bPatternSoundDependencyDirty =
			Is_PatternSoundDraftDirty(SoundDependencyStatus);
	}
	if (!SoundDependencyStatus.empty())
		m_strSoundStatus = SoundDependencyStatus;
	const bool_t bPatternMutationAdmitted = bMutationAdmitted;

	if (m_PendingLocalPreview)
	{
		const auto pending = *m_PendingLocalPreview;
		if (!pPattern || !bLocalPreviewAdmitted || pPattern->strPatternId != pending.patternId ||
			iPatternViewDraftGeneration != pending.draftGeneration ||
			m_iCanonicalDisplayGeneration != pending.canonicalGeneration || m_ePreviewPath != pending.path ||
			CEffectCatalog::Get_RuntimeRevision() != pending.v1Revision ||
			CEffectV2Catalog::Get().Get_Revision() != pending.v2Revision ||
			IsValtanServerPlaybackBusy(m_pValtanBossTool))
		{
			m_PendingLocalPreview.reset();
			m_strStatus = "Preview preparation cancelled because its selection or draft changed.";
		}
		else
		{
			const auto probe = CEffectPresentationService::Get_ProductCuePreparationProbe(pending.effectIds);
			if (probe.bSettled || probe.iFailedCount || probe.iUnavailableCount || !probe.strBlockingFailure.empty())
			{
				m_PendingLocalPreview.reset();
				if (Play_EffectivePreview(*pPattern, m_strStatus) && (pending.positionMs || pending.pause))
					(void)m_pAnimationTool->Seek_ValtanCompositionPattern(
						pending.patternId, pending.positionMs, pending.pause, m_strStatus);
			}
		}
	}
	m_pWorkbenchFramePattern = pPattern;
	m_pWorkbenchFrameStage = pStage;
	m_iWorkbenchFrameDraftGeneration = iPatternViewDraftGeneration;
	m_bWorkbenchFrameLocalPreviewAdmitted = bLocalPreviewAdmitted;
	m_bWorkbenchFrameMutationAdmitted = bMutationAdmitted;
	m_bWorkbenchFramePatternMutationAdmitted = bPatternMutationAdmitted;
	m_bHistoryWindowFocused = false;
	Begin_EditHistory();
	m_bWorkbenchFrameActive = true;
}

void Client::CValtanActionWorkbench::Render_WorkbenchPane(
	const COMPOSITION_WORKBENCH_PANE pane)
{
	if (!m_bWorkbenchFrameActive)
		return;
	m_bSharedPaneRendering = true;
	switch (pane)
	{
	case COMPOSITION_WORKBENCH_PANE::TOOLBAR:
		(void)Render_Toolbar(m_pWorkbenchFramePattern,
			m_bWorkbenchFramePatternMutationAdmitted);
		break;
	case COMPOSITION_WORKBENCH_PANE::SEQUENCER:
		Render_Timeline(m_pWorkbenchFramePattern,
			m_bWorkbenchFrameLocalPreviewAdmitted,
			m_bWorkbenchFrameMutationAdmitted,
			m_bWorkbenchFramePatternMutationAdmitted);
		break;
	case COMPOSITION_WORKBENCH_PANE::PATTERNS:
		Render_PendingPatternSelectionModal();
		if (m_bObjectWorkspace) Render_WorldObjectResources();
		else Render_PatternsPane(m_pWorkbenchFramePattern, m_pWorkbenchFrameStage,
			m_bWorkbenchFramePatternMutationAdmitted);
		break;
	case COMPOSITION_WORKBENCH_PANE::RESOURCES:
		Render_ResourcesPane(m_pWorkbenchFramePattern, m_pWorkbenchFrameStage,
			m_bWorkbenchFrameMutationAdmitted,
			m_bWorkbenchFramePatternMutationAdmitted);
		break;
	case COMPOSITION_WORKBENCH_PANE::DETAILS:
		Render_Details(m_pWorkbenchFramePattern, m_pWorkbenchFrameStage,
			m_bWorkbenchFrameMutationAdmitted,
			m_bWorkbenchFramePatternMutationAdmitted);
		break;
	case COMPOSITION_WORKBENCH_PANE::PREVIEW:
		Render_Preview(m_pWorkbenchFramePattern,
			m_bWorkbenchFrameLocalPreviewAdmitted,
			m_bWorkbenchFramePatternMutationAdmitted);
		break;
	case COMPOSITION_WORKBENCH_PANE::BOSS_PATTERN:
		Render_BossPatternPane(m_pWorkbenchFramePattern,
			m_iWorkbenchFrameDraftGeneration, m_bWorkbenchFrameMutationAdmitted,
			m_bWorkbenchFramePatternMutationAdmitted);
		break;
	case COMPOSITION_WORKBENCH_PANE::COUNT:
		break;
	}
	m_bSharedPaneRendering = false;
}

COMPOSITION_WORKBENCH_VIEW_REQUEST
Client::CValtanActionWorkbench::Consume_WorkbenchViewRequest()
{
	COMPOSITION_WORKBENCH_VIEW_REQUEST request;
	request.showResources = m_bResourcesWindowFocusRequested || m_bResourcesWindowExpandRequested;
	request.focusResources = m_bResourcesWindowFocusRequested;
	request.expandResources = m_bResourcesWindowExpandRequested;
	m_bResourcesWindowFocusRequested = false;
	m_bResourcesWindowExpandRequested = false;
	if (m_iRequestedPatternTab >= 0 &&
		m_iRequestedPatternTab != m_iWorkbenchReportedPatternTab)
	{
		request.showPatterns = true;
		request.focusPatterns = true;
	}
	m_iWorkbenchReportedPatternTab = m_iRequestedPatternTab;
	if (m_bTimelineMaximized != m_bWorkbenchReportedTimelineMaximized)
	{
		request.maximizeSequencer = m_bTimelineMaximized;
		request.restoreSequencer = !m_bTimelineMaximized;
		m_bWorkbenchReportedTimelineMaximized = m_bTimelineMaximized;
	}
	request.resetLayout = m_bResetLayoutRequested || m_bApplyResetLayoutThisFrame;
	m_bResetLayoutRequested = false;
	m_bApplyResetLayoutThisFrame = false;
	return request;
}

Client::COMPOSITION_TRANSFER Client::CValtanActionWorkbench::Capture_CompositionSelection(std::string& status)
{
	if (!m_pBalanceTool || !Can_MutateValtanView(m_eAdmission))
	{ status = "Copy requires an admitted Valtan authoring session."; return {}; }
	VALTAN_PATTERN_VIEW pattern;
	if (!m_pBalanceTool->Get_ValtanPatternDraft(m_strSelectedPatternId, pattern, status)) return {};
	std::set<std::string> copiedStageIds;
	for (const auto& selected : m_TimelineSelection)
		if (selected.strPatternId == pattern.strPatternId && selected.eOwner == DETAIL_OWNER::GAMEPLAY_STAGE &&
			selected.strStableId == selected.strStageId) copiedStageIds.insert(selected.strStageId);
	if (m_TimelineSelection.empty() && m_eDetailOwner == DETAIL_OWNER::GAMEPLAY_STAGE &&
		m_strSelectedStableId == m_strSelectedStageId) copiedStageIds.insert(m_strSelectedStageId);
	if (!copiedStageIds.empty())
	{
		Ensure_TimelineCache(&pattern);
		bool_t dirty = false;
		const auto* sounds = m_pAnimationTool ? m_pAnimationTool->Get_ValtanCompositionPatternSoundDraft(dirty, status) : nullptr;
		const auto effects = CEffectV2Catalog::Get().Get_Snapshot();
		if (!sounds || !effects || !effects->Can_MutateBossValtanBindings() || !m_bPatternShakesReady)
		{ status = "Stage Copy requires the admitted Sound, Effect and Camera Shake resource views."; return {}; }
		auto batch = std::make_shared<VALTAN_STAGE_BATCH>();
		batch->firstStageMs = (std::numeric_limits<uint32_t>::max)();
		for (const auto& source : pattern.Stages)
		{
			if (!copiedStageIds.contains(source.strStageId)) continue;
			const auto box = std::find_if(m_TimelineItems.begin(), m_TimelineItems.end(), [&](const auto& item) {
				return item.eLane == TIMELINE_LANE::STAGE && item.strStageId == source.strStageId; });
			if (box == m_TimelineItems.end()) { status = "A selected Stage is outside the current timeline."; return {}; }
			if ((pattern.ServerMotion && pattern.ServerMotion->strTravelStageId == source.strStageId) ||
				std::any_of(pattern.WorldEventTriggerRefs.begin(), pattern.WorldEventTriggerRefs.end(), [&](const auto& ref) { return ref.strStageId == source.strStageId; }) ||
				std::any_of(pattern.Reactions.begin(), pattern.Reactions.end(), [&](const auto& ref) { return ref.strStageId == source.strStageId; }))
			{ status = "Stage " + source.strStageId + " is referenced by Pattern motion, World or reaction gameplay. Select its resource boxes for Copy instead."; return {}; }
			VALTAN_STAGE_BATCH::ITEM item; item.stage = source;
			for (const auto& cue : m_PatternShakes.Cues)
				if (cue.strPatternId == pattern.strPatternId && cue.strStageId == source.strStageId)
				{
					if (cue.strActionId != source.strActionId) { status = "Stage Copy found a stale Camera Shake owner; clipboard preserved."; return {}; }
					item.shakes.push_back(cue);
				}
			for (const auto& cue : sounds->Cues)
				if (cue.strPatternId == pattern.strPatternId && cue.strStageId == source.strStageId && cue.strActionId == source.strActionId)
				{ item.sounds.push_back(cue); item.sounds.back().ResolvedAssetIds.clear(); }
			for (const auto& cue : effects->Get_BossValtanBindings())
				if (cue.strPatternId == pattern.strPatternId && cue.strStageId == source.strStageId && cue.strActionId == source.strActionId) item.effects.push_back(cue);
			batch->firstStageMs = (std::min)(batch->firstStageMs, box->iStartMs);
			batch->items.push_back(std::move(item));
		}
		if (batch->items.size() != copiedStageIds.size()) { status = "A selected Stage identity is stale."; return {}; }
		std::vector<TIMELINE_SELECTION> extras;
		batch->extrasFirstMs = (std::numeric_limits<uint32_t>::max)();
		for (const auto& selected : m_TimelineSelection)
		{
			if (copiedStageIds.contains(selected.strStageId)) continue;
			const auto box = std::find_if(m_TimelineItems.begin(), m_TimelineItems.end(), [&](const auto& item) {
				return Timeline_SelectionKey(item) == selected; });
			if (box == m_TimelineItems.end()) { status = "A selected resource identity is stale."; return {}; }
			batch->extrasFirstMs = (std::min)(batch->extrasFirstMs, box->iStartMs);
			extras.push_back(selected);
		}
		if (!extras.empty())
		{
			const auto savedSelection = m_TimelineSelection;
			const auto savedStage = m_strSelectedStageId, savedStable = m_strSelectedStableId;
			const auto savedOwner = m_eDetailOwner;
			m_TimelineSelection = extras;
			m_strSelectedStageId = extras.back().strStageId; m_strSelectedStableId = extras.back().strStableId; m_eDetailOwner = extras.back().eOwner;
			batch->extras = Capture_CompositionSelection(status);
			m_TimelineSelection = savedSelection; m_strSelectedStageId = savedStage; m_strSelectedStableId = savedStable; m_eDetailOwner = savedOwner;
			if (!batch->extras) return {};
		}
		batch->label = std::to_string(batch->items.size()) + " Stage(s) with all owned resources";
		status = "Copied " + batch->label + (batch->extras ? " and additional selected boxes." : ".");
		return batch;
	}

	if (m_TimelineSelection.size() > 1u)
	{
		if (m_TimelineSelection.size() > 128u)
		{ status = "Copy accepts at most 128 Animation, Effect, Sound and Collider boxes."; return {}; }
		Ensure_TimelineCache(&pattern);
		auto batch = std::make_shared<VALTAN_PRESENTATION_BATCH>();
		uint32_t firstMs = (std::numeric_limits<uint32_t>::max)();
		const auto snapshot = CEffectV2Catalog::Get().Get_Snapshot();
		for (const auto& key : m_TimelineSelection)
		{
			const auto item = std::find_if(m_TimelineItems.begin(), m_TimelineItems.end(), [&](const auto& row) {
				return row.eLane != TIMELINE_LANE::STAGE && Timeline_SelectionKey(row) == key; });
			const auto sourceStage = std::find_if(pattern.Stages.begin(), pattern.Stages.end(),
				[&](const auto& row) { return row.strStageId == key.strStageId; });
			if (key.strPatternId != pattern.strPatternId || item == m_TimelineItems.end() ||
				(!item->bEditable && !item->bEffectV2Binding) ||
				sourceStage == pattern.Stages.end() || (key.eOwner != DETAIL_OWNER::ANIMATION && key.eOwner != DETAIL_OWNER::EFFECT && key.eOwner != DETAIL_OWNER::SOUND &&
				 !(key.eOwner == DETAIL_OWNER::GAMEPLAY_STAGE && key.strStableId == key.strStageId + "/collider")))
			{ status = "Group Copy requires current editable Animation, Effect, Sound or Damage Collider boxes; the clipboard was preserved."; return {}; }
			VALTAN_PRESENTATION_BATCH::ITEM entry;
			auto& value = entry.value;
			value.owner = key.eOwner; value.patternId = key.strPatternId; value.stageId = key.strStageId;
			value.stableId = key.strStableId; value.sourceRevision = m_strPinnedAuthoringSourceRevision;
			entry.offsetMs = item->iStartMs;
			if (key.eOwner == DETAIL_OWNER::ANIMATION)
			{
				CBalanceTool::PATTERN_STAGE_EDIT source;
				if (!m_pBalanceTool->Get_ValtanStageDraft(pattern.strPatternId, sourceStage->strStageId, source, status)) return {};
				const auto slot = std::find_if(source.animationSlots.begin(), source.animationSlots.end(),
					[&](const auto& candidate) { return candidate.clipOccurrenceId == key.strStableId; });
				if (slot == source.animationSlots.end() || !source.animationEditable || !slot->playMs ||
					slot->repeatUntilStageEnd || !std::isfinite(slot->playRate) || slot->playRate <= 0.0)
				{ status = "Animation Copy requires finite editable slots; copy the whole Stage to retain a loop."; return {}; }
				value.animation = *slot; value.animationSources = pattern.PresentationSources;
			}
			else if (key.eOwner == DETAIL_OWNER::GAMEPLAY_STAGE)
			{
				CBalanceTool::PATTERN_STAGE_EDIT collider;
				uint32_t first = 0u, last = 0u;
				if (!m_pBalanceTool->Get_ValtanStageDraft(pattern.strPatternId, sourceStage->strStageId, collider, status) ||
					!collider.attackContacts.empty() || collider.playerResponse != "DAMAGE" ||
					!ResolveValtanColliderScheduleEndpoints(collider, first, last, status))
				{ status = "Copy requires an ordinary Damage collider; per-contact and Grab owners keep their typed editors. " + status; return {}; }
				value.collider = std::move(collider);
			}
			else if (key.eOwner == DETAIL_OWNER::SOUND)
			{
				bool dirty = false;
				const auto* sounds = m_pAnimationTool ? m_pAnimationTool->Get_ValtanCompositionPatternSoundDraft(dirty, status) : nullptr;
				if (!sounds) return {};
				for (const auto& cue : sounds->Cues)
					if (cue.strPatternId == key.strPatternId && cue.strStageId == key.strStageId &&
						cue.strActionId == sourceStage->strActionId && cue.strOccurrenceId == key.strStableId) value.sound = cue;
				if (!value.sound) { status = "A selected Sound occurrence no longer exists."; return {}; }
				value.sound->ResolvedAssetIds.clear();
			}
			else if (item->bEffectV2Binding)
			{
				if (!snapshot || !snapshot->Can_MutateBossValtanBindings())
				{ status = "The V2 binding owner is not admitted for authoring."; return {}; }
				if (snapshot) for (const auto& cue : snapshot->Get_BossValtanBindings())
					if (cue.strPatternId == key.strPatternId && cue.strStageId == key.strStageId &&
						BuildEffectV2BindingStableId(cue, cue.strClipOccurrenceId) == key.strStableId) value.effectV2 = cue;
				if (!value.effectV2) { status = "A selected V2 Effect binding no longer exists."; return {}; }
			}
			else
			{
				for (const auto& cue : sourceStage->ProductCues)
					if (cue.strOccurrenceId == key.strStableId) value.effect = cue;
				if (!value.effect)
				{ status = "Group Copy requires exact V1 Effect invocations."; return {}; }
				const auto sourceClip = std::find_if(sourceStage->ClipOccurrences.begin(), sourceStage->ClipOccurrences.end(),
					[&](const auto& clip) { return clip.strClipOccurrenceId == value.effect->strClipOccurrenceId; });
				if (!value.effect->bUsesStageClock && (sourceClip == sourceStage->ClipOccurrences.end() ||
					!std::isfinite(sourceClip->fPlayRate) || sourceClip->fPlayRate <= 0.f))
				{ status = "A copied linked Effect has no valid Animation clock."; return {}; }
				const double rate = value.effect->bUsesStageClock ? 1.0 : double(sourceClip->fPlayRate);
				if (value.effect->bHasSourceEnd)
				{
					if (value.effect->iSourceEndMs <= value.effect->iSourceStartMs)
					{ status = "A copied Effect has an invalid explicit end."; return {}; }
					entry.effectDurationWallMs = (value.effect->iSourceEndMs - value.effect->iSourceStartMs) / rate;
				}
			}
			firstMs = (std::min)(firstMs, entry.offsetMs);
			batch->items.push_back(std::move(entry));
		}
		for (auto& entry : batch->items) entry.offsetMs -= firstMs;
		batch->label = std::to_string(batch->items.size()) + " Valtan Animation/Effect/Sound/Collider boxes";
		status = "Copied " + batch->label + " with relative timing and authored settings.";
		return batch;
	}
	const auto stage = std::find_if(pattern.Stages.begin(), pattern.Stages.end(),
		[&](const auto& row) { return row.strStageId == m_strSelectedStageId; });
	if (stage == pattern.Stages.end()) { status = "Select an editable timeline occurrence."; return {}; }
	auto copy = std::make_shared<VALTAN_EDIT_TRANSFER>();
	copy->owner = m_eDetailOwner; copy->patternId = pattern.strPatternId; copy->stageId = stage->strStageId;
	copy->sourceRevision = m_strPinnedAuthoringSourceRevision; copy->stableId = m_strSelectedStableId;
	copy->label = m_strSelectedStableId;
	const auto capture = [&](const auto& rows, auto& destination, auto id) {
		const auto found = std::find_if(rows.begin(), rows.end(), [&](const auto& row) { return id(row) == copy->stableId; });
		if (found == rows.end()) return false;
		destination = *found; return true;
	};
	if (copy->owner == DETAIL_OWNER::ANIMATION)
	{
		CBalanceTool::PATTERN_STAGE_EDIT draft;
		if (!m_pBalanceTool->Get_ValtanStageDraft(pattern.strPatternId, stage->strStageId, draft, status)) return {};
		if (!capture(draft.animationSlots, copy->animation, [](const auto& x) { return x.clipOccurrenceId; }) ||
			copy->animation->repeatUntilStageEnd || !copy->animation->playMs)
		{ status = "Copy requires one finite Animation occurrence."; return {}; }
		copy->animationSources = pattern.PresentationSources;
	}
	else if (copy->owner == DETAIL_OWNER::EFFECT)
	{
		if (!capture(stage->ProductCues, copy->effect, [](const auto& x) { return x.strOccurrenceId; }))
		{
			const auto snapshot = CEffectV2Catalog::Get().Get_Snapshot();
			if (snapshot) for (const auto& binding : snapshot->Get_BossValtanBindings())
				if (binding.strPatternId == pattern.strPatternId && binding.strStageId == stage->strStageId &&
					BuildEffectV2BindingStableId(binding, binding.strClipOccurrenceId) == copy->stableId) copy->effectV2 = binding;
			if (!copy->effectV2) { status = "Select one exact Effect occurrence."; return {}; }
		}
		COMPOSITION_EFFECT_ITEM item;
		bool portable = false;
		if (copy->effect)
		{
			const auto& cue = *copy->effect;
			copy->startMs = cue.bUsesStageClock ? cue.iStageOffsetMs : Resolve_ClipSourceToStageMs(*stage, cue.strClipOccurrenceId, cue.iSourceStartMs);
			portable = cue.strAnchorSlotId == "root" && cue.strRepeatPolicy == "once" &&
				cue.eScalePolicy == VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE &&
				cue.strV1EffectAssetId.empty() && ResolveEffectPlaybackOffsetMs(cue) == 0u;
			item.resourceId = cue.strEffectAssetId;
			item.followOwner = cue.eFollowPolicy == EFFECT_FOLLOW_POLICY::FOLLOW;
			item.position = {cue.LocalTransform.vPosition.x, cue.LocalTransform.vPosition.y, cue.LocalTransform.vPosition.z};
			item.rotationDegrees = {cue.LocalTransform.vRotationDegrees.x, cue.LocalTransform.vRotationDegrees.y, cue.LocalTransform.vRotationDegrees.z};
			item.scale = {cue.LocalTransform.vScale.x, cue.LocalTransform.vScale.y, cue.LocalTransform.vScale.z};
			if (cue.bHasSourceEnd)
				item.durationMs = (cue.bUsesStageClock ? cue.iSourceEndMs :
					Resolve_ClipSourceToStageMs(*stage, cue.strClipOccurrenceId, cue.iSourceEndMs)) - copy->startMs;
		}
		else
		{
			const auto& cue = *copy->effectV2;
			copy->startMs = cue.eClockBasis == EFFECT_V2_CLOCK_BASIS::STAGE ? cue.iStartMs :
				Resolve_ClipSourceToStageMs(*stage, cue.strClipOccurrenceId, cue.iStartMs);
			portable = cue.strAnchorSlotId == "b_effectroot" && cue.eRepeatPolicy == EFFECT_V2_REPEAT_POLICY::ONCE &&
				cue.eStopPolicy == EFFECT_V2_STOP_POLICY::NATURAL && cue.eRotationBasis != EFFECT_V2_ROTATION_BASIS::SLOT;
			item.resourceKind = cue.eResourceKind == EFFECT_V2_RESOURCE_KIND::GROUP ? "GROUP" : "LEAF";
			item.resourceId = cue.strResourceId;
			item.followOwner = cue.eFollowPolicy == EFFECT_V2_FOLLOW_POLICY::FOLLOW_SLOT;
			item.inheritOwnerRotation = cue.eRotationBasis == EFFECT_V2_ROTATION_BASIS::TARGET_YAW;
			item.position = {cue.LocalTransform.vTranslation.x, cue.LocalTransform.vTranslation.y, cue.LocalTransform.vTranslation.z};
			item.rotationDegrees = {cue.LocalTransform.vRotation.x, cue.LocalTransform.vRotation.y, cue.LocalTransform.vRotation.z};
			item.scale = {cue.LocalTransform.vScale.x, cue.LocalTransform.vScale.y, cue.LocalTransform.vScale.z};
		}
		if (portable)
		{
			auto result = std::make_shared<VALTAN_EFFECT_TRANSFER>();
			result->native = copy; result->items.push_back(item); result->label = copy->label;
			status = "Copied Effect values and its reusable resource reference."; return result;
		}
	}
	else if (copy->owner == DETAIL_OWNER::SOUND)
	{
		bool dirty = false;
		const auto* document = m_pAnimationTool ? m_pAnimationTool->Get_ValtanCompositionPatternSoundDraft(dirty, status) : nullptr;
		if (!document || !capture(document->Cues, copy->sound, [](const auto& x) { return x.strOccurrenceId; }) ||
			copy->sound->strPatternId != pattern.strPatternId || copy->sound->strStageId != stage->strStageId)
		{ status = "Select one exact Sound occurrence."; return {}; }
	}
	else if (copy->owner == DETAIL_OWNER::SCENE_PROFILE)
	{
		if (!capture(stage->SceneProfileOccurrences, copy->scene, [](const auto& x) { return x.strOccurrenceId; })) return {};
	}
	else if (copy->owner == DETAIL_OWNER::LIGHT)
	{
		if (!capture(stage->LightOccurrences, copy->light, [](const auto& x) { return x.strOccurrenceId; })) return {};
	}
	else if (copy->owner == DETAIL_OWNER::CAMERA)
	{
		if (!capture(stage->CameraInvocations, copy->camera, [](const auto& x) { return x.strCameraInvocationId; }))
		{ status = "Only explicit Stage Camera invocations can be copied."; return {}; }
	}
	else if (copy->owner == DETAIL_OWNER::COMBAT_OBJECT)
	{
		if (!capture(stage->CombatObjectEffects, copy->summon, [](const auto& x) { return x.strCombatObjectArchetypeId; })) return {};
	}
	else if (copy->owner == DETAIL_OWNER::GAMEPLAY_STAGE)
	{
		if (copy->stableId == stage->strStageId + "/collider")
		{
			CBalanceTool::PATTERN_STAGE_EDIT collider;
			uint32_t first = 0u, last = 0u;
			if (!m_pBalanceTool->Get_ValtanStageDraft(pattern.strPatternId, stage->strStageId, collider, status) ||
				!collider.attackContacts.empty() || collider.playerResponse != "DAMAGE" ||
				!ResolveValtanColliderScheduleEndpoints(collider, first, last, status))
			{ status = "Copy requires an ordinary Damage collider; per-contact and Grab owners keep their typed editors. " + status; return {}; }
			copy->collider = std::move(collider);
			status = "Copied Collider geometry, damage, reaction and relative hit timing.";
			return copy;
		}
		const auto action = std::find_if(stage->Actions.begin(), stage->Actions.end(), [&](const auto& x) {
			return stage->strStageId + "/action/" + x.strKind + "/" + x.strTargetId + "/" + x.strTrigger == copy->stableId; });
		if (action != stage->Actions.end() && action->strKind == "TRIGGER_WORLD_EVENT_SET")
		{ status = "World destruction events have one owner. Use Resources > World > Move to Stage."; return {}; }
		if (action == stage->Actions.end() || action->strTargetId == "boss.flag.counterable")
		{ status = "Stage topology, branch, Counter and motion owners require their typed editors; select a reusable Logic action."; return {}; }
		copy->actions.push_back(*action);
		if (action->strKind.starts_with("SET_")) for (const auto& other : stage->Actions)
			if (other.strKind == action->strKind && other.strTargetId == action->strTargetId && other.strTrigger != action->strTrigger)
				copy->actions.push_back(other);
	}
	else { status = "This timeline owner has no transferable occurrence."; return {}; }
	status = "Copied the exact Valtan occurrence values.";
	return copy;
}

bool Client::CValtanActionWorkbench::Execute_CompositionEdit(COMPOSITION_EDIT_COMMAND command, std::string& status)
{
	if (m_bCompositionResourcesFocused && command != COMPOSITION_EDIT_COMMAND::PASTE)
	{
		status = m_strStatus = "Use Copy Resource or drag the resource row; timeline Copy and Duplicate require a selected timeline box.";
		return false;
	}

	const bool result = Dispatch_CompositionEdit(command,
		[&](std::string& message) { return Capture_CompositionSelection(message); },
		[&](const auto& value, std::string& message) { return Insert_CompositionTransfer(value, message); },
		[&](std::string& message) {
			auto value = Capture_CompositionSelection(message);
			if (!value) return false;
			if (m_bWorkbenchFrameActive)
			{
				if (m_PendingCompositionPaste) { message = "A paste is already queued."; return false; }
				m_PendingCompositionPaste = PENDING_COMPOSITION_PASTE{value, m_strSelectedPatternId, m_strSelectedStageId, m_iPlayheadMs, true};
				return true;
			}
			return Apply_CompositionTransfer(value, m_strSelectedPatternId, m_strSelectedStageId, m_iPlayheadMs, true, message);
		}, status);
	m_strStatus = status;
	return result;
}

bool Client::CValtanActionWorkbench::Insert_CompositionTransfer(const COMPOSITION_TRANSFER& transfer, std::string& status)
{
	if (const auto animation = std::dynamic_pointer_cast<const COMPOSITION_ANIMATION_TRANSFER>(transfer))
		return Append_CompositionAnimationResource(animation->resource, false, status);
	if (m_bWorkbenchFrameActive)
	{
		if (m_PendingCompositionPaste) { status = "A paste is already queued."; return false; }
		m_PendingCompositionPaste = PENDING_COMPOSITION_PASTE{transfer, m_strSelectedPatternId, m_strSelectedStageId, m_iPlayheadMs, false};
		status = "Paste queued until the current panes finish."; return true;
	}
	const bool result = Apply_CompositionTransfer(transfer, m_strSelectedPatternId, m_strSelectedStageId, m_iPlayheadMs, false, status);
	m_strStatus = status; return result;
}

bool Client::CValtanActionWorkbench::Apply_CompositionTransfer(const COMPOSITION_TRANSFER& transfer,
	const std::string& patternId, const std::string& stageId, const uint32_t playheadMs,
	const bool duplicate, std::string& status)
{
	HISTORY_SCOPE history(*this);
	if (!m_pBalanceTool || !Can_MutateValtanView(m_eAdmission) || m_pBalanceTool->Is_ValtanSaveJobBlockingAuthoring() ||
		m_pBalanceTool->Is_ServerRuntimeSetPublishRunning() ||
		(m_pAnimationTool && m_pAnimationTool->Is_ValtanCompositionPatternTransactionActive()))
	{ status = "Paste requires an admitted, idle Valtan draft."; return false; }
	VALTAN_PATTERN_VIEW pattern;
	if (!m_pBalanceTool->Get_ValtanPatternDraft(patternId, pattern, status)) return false;
	auto stage = std::find_if(pattern.Stages.begin(), pattern.Stages.end(), [&](const auto& row) { return row.strStageId == stageId; });
	if (stage == pattern.Stages.end()) { status = "Select a destination Stage."; return false; }
	std::vector<const VALTAN_STAGE_VIEW*> path;
	if (!CValtanPatternTree::Build_PreviewStagePath(pattern, m_ePreviewPath, path, status)) return false;
	uint32_t stageStart = 0u;
	bool foundStage = false;
	for (const auto* row : path) { if (row->strStageId == stageId) { foundStage = true; break; } stageStart += row->iDurationMs; }
	if (!foundStage || !stage->iDurationMs) { status = "The destination Stage is outside the active timeline path."; return false; }
	const uint32_t localStart = playheadMs >= stageStart && playheadMs - stageStart < stage->iDurationMs ? playheadMs - stageStart : 0u;
	if (const auto stages = std::dynamic_pointer_cast<const VALTAN_STAGE_BATCH>(transfer))
	{
		if (!m_pAnimationTool || stages->items.empty() || !m_bPatternShakesReady) { status = "Stage Paste requires a nonempty clipboard and the Sound owner."; return false; }
		const uint64_t insertionStart = uint64_t(stageStart) + stage->iDurationMs;
		const auto savedSelection = m_TimelineSelection;
		auto candidateShakes = m_PatternShakes;
		const auto savedPattern = m_strSelectedPatternId, savedStage = m_strSelectedStageId, savedStable = m_strSelectedStableId;
		const auto savedOwner = m_eDetailOwner;
		const auto savedEffectIdentity = m_strEffectEditIdentity;
		const auto savedDirty = m_bAuthoringDraftDirty;
		std::vector<TIMELINE_SELECTION> createdSelection;
		const bool applied = CEffectV2Catalog::Get().Apply_BossValtanBindingDraftTransaction([&](std::string& message) {
			return m_pAnimationTool->Apply_ValtanCompositionPatternSoundDraftTransaction([&](std::string& soundStatus) {
				return m_pBalanceTool->Apply_ValtanCompositionDraftTransaction([&](std::string& diagnostic) {
					std::string after = stageId;
					std::vector<EFFECT_V2_BINDING> effects;
					for (const auto& item : stages->items)
					{
						VALTAN_STAGE_VIEW created;
						if (!m_pBalanceTool->Insert_ValtanStageCopyAfter(patternId, after, item.stage, created, diagnostic)) return false;
						after = created.strStageId;
						std::map<std::string, std::string> clips;
						for (std::size_t index = 0u; index < item.stage.ClipOccurrences.size(); ++index)
							clips[item.stage.ClipOccurrences[index].strClipOccurrenceId] = created.ClipOccurrences[index].strClipOccurrenceId;
						VALTAN_PATTERN_VIEW current;
						if (!m_pBalanceTool->Get_ValtanPatternDraft(patternId, current, diagnostic)) return false;
						const auto stagePosition = std::find_if(current.Stages.begin(), current.Stages.end(), [&](const auto& row) { return row.strStageId == created.strStageId; });
						if (!CValtanPatternShakeCueDocument::Append_StageCopyDraft(candidateShakes, item.shakes, item.stage,
							created, patternId, static_cast<uint32_t>(stagePosition - current.Stages.begin()), diagnostic)) return false;
						for (const auto& cue : item.sounds)
						{
							const auto clip = clips.find(cue.strClipOccurrenceId);
							if (clip == clips.end()) { diagnostic = "A copied Sound references a missing source clip."; return false; }
							VALTAN_PATTERN_SOUND_CUE_ROW_ID row;
							if (!m_pAnimationTool->Add_ValtanCompositionPatternSound(current, created, clip->second,
								cue.strSoundEvent, cue.iStartMs, cue.eRepeatPolicy, row, diagnostic,
								cue.bHasPlaybackOffset ? std::optional<uint32_t>{cue.iPlaybackOffsetMs} : std::nullopt,
								cue.bHasPlaybackDuration ? std::optional<uint32_t>{cue.iPlaybackDurationMs} : std::nullopt)) return false;
						}
						for (auto cue : item.effects)
						{
							cue.strPatternId = patternId; cue.strStageId = created.strStageId; cue.strActionId = created.strActionId;
							if (cue.eClockBasis == EFFECT_V2_CLOCK_BASIS::CLIP_OCCURRENCE)
							{
								const auto clip = clips.find(cue.strClipOccurrenceId);
								if (clip == clips.end()) { diagnostic = "A copied Effect references a missing source clip."; return false; }
								cue.strClipOccurrenceId = clip->second;
							}
							if (!Validate_EffectV2BindingClock(created, cue, diagnostic)) return false;
							effects.push_back(std::move(cue));
						}
						createdSelection.push_back({patternId, created.strStageId, created.strStageId, DETAIL_OWNER::GAMEPLAY_STAGE});
					}
					std::vector<std::string> effectIds;
					if (!effects.empty() && !CEffectV2Catalog::Get().Stage_AppendBossValtanBindings(effects, effectIds, diagnostic)) return false;
					if (stages->extras)
					{
						const int64_t at = static_cast<int64_t>(insertionStart) + int64_t(stages->extrasFirstMs) - stages->firstStageMs;
						if (at < 0 || at > 600000) { diagnostic = "The additional copied boxes fall outside the destination timeline."; return false; }
						VALTAN_PATTERN_VIEW current;
						if (!m_pBalanceTool->Get_ValtanPatternDraft(patternId, current, diagnostic)) return false;
						uint64_t cursor = 0u; std::string target = after;
						for (const auto& candidate : current.Stages)
						{
							if (uint64_t(at) >= cursor && uint64_t(at) - cursor < candidate.iDurationMs) { target = candidate.strStageId; break; }
							cursor += candidate.iDurationMs;
						}
						if (!Apply_CompositionTransfer(stages->extras, patternId, target, static_cast<uint32_t>(at), false, diagnostic)) return false;
						createdSelection.insert(createdSelection.end(), m_TimelineSelection.begin(), m_TimelineSelection.end());
					}
					return true;
				}, soundStatus);
			}, message);
		}, status);
		if (!applied)
		{
			m_TimelineSelection = savedSelection; m_strSelectedPatternId = savedPattern; m_strSelectedStageId = savedStage;
			m_strSelectedStableId = savedStable; m_eDetailOwner = savedOwner;
			m_strEffectEditIdentity = savedEffectIdentity; m_bAuthoringDraftDirty = savedDirty;
			Invalidate_EffectivePatternCache(); Invalidate_TimelineCache();
			status = "Stage Paste rejected; all destination drafts were preserved. " + status;
			return false;
		}
		m_PatternShakes = std::move(candidateShakes);
		m_TimelineSelection = std::move(createdSelection);
		const auto& selected = m_TimelineSelection.front();
		m_strSelectedPatternId = patternId; m_strSelectedStageId = selected.strStageId;
		m_strSelectedStableId = selected.strStableId; m_eDetailOwner = selected.eOwner;
		m_strEffectEditIdentity.clear(); m_BossPatternOutcomeOverrides.clear(); ++m_iBossPatternRouteGeneration;
		m_iEffectV2CatalogRevision = CEffectV2Catalog::Get().Get_Revision();
		Invalidate_EffectivePatternCache(); Invalidate_TimelineCache();
		m_bAuthoringDraftDirty = m_pBalanceTool->Is_ValtanDraftDirty();
		status = "Pasted " + std::to_string(stages->items.size()) + " Stage(s) with Animation, Effect, Sound and Collider; all copied IDs are independent. Save when ready.";
		return true;
	}

	if (const auto batch = std::dynamic_pointer_cast<const VALTAN_PRESENTATION_BATCH>(transfer))
	{
		if (batch->items.empty() || batch->items.size() > 128u)
		{ status = "The copied Animation/Effect/Sound/Collider batch is empty or too large."; return false; }
		const bool hasSound = std::any_of(batch->items.begin(), batch->items.end(), [](const auto& item) { return item.value.sound.has_value(); });
		if (hasSound && !m_pAnimationTool) { status = "The destination Pattern Sound owner is unavailable."; return false; }
		std::vector<TIMELINE_SELECTION> nextSelection;
		std::vector<EFFECT_V2_BINDING> v2Bindings;
		std::vector<std::string> v2Ids;
		std::vector<std::size_t> v2SelectionIndexes;
		nextSelection.reserve(batch->items.size()); v2Bindings.reserve(batch->items.size()); v2SelectionIndexes.reserve(batch->items.size());
		struct COPIED_ANIMATION { std::string occurrenceId; uint32_t startMs = 0u; CBalanceTool::ANIMATION_SLOT_EDIT source; };
		std::unordered_map<std::string, COPIED_ANIMATION> copiedAnimations;
		const auto sourceKey = [](const auto& value, const std::string& clipId)
			{ return value.patternId + "/" + value.stageId + "/" + clipId; };
		std::vector<const VALTAN_PRESENTATION_BATCH::ITEM*> animationItems;
		for (const auto& item : batch->items) if (item.value.animation) animationItems.push_back(&item);
		std::stable_sort(animationItems.begin(), animationItems.end(), [](const auto* left, const auto* right) { return left->offsetMs < right->offsetMs; });
		int64_t animationBatchBase = 0;
		const auto mappedAnimation = [&](const auto& value) -> const COPIED_ANIMATION* {
			std::string clipId;
			if (value.effect && !value.effect->bUsesStageClock) clipId = value.effect->strClipOccurrenceId;
			else if (value.sound) clipId = value.sound->strClipOccurrenceId;
			else if (value.effectV2 && value.effectV2->eClockBasis == EFFECT_V2_CLOCK_BASIS::CLIP_OCCURRENCE) clipId = value.effectV2->strClipOccurrenceId;
			if (clipId.empty()) return nullptr;
			const auto found = copiedAnimations.find(sourceKey(value, clipId));
			return found == copiedAnimations.end() ? nullptr : &found->second;
		};
		const auto originalSourceMs = [](const auto& value) -> uint32_t {
			return value.effect ? value.effect->iSourceStartMs : value.sound ? value.sound->iStartMs : value.effectV2 ? value.effectV2->iStartMs : 0u;
		};
		const auto mixedAnimationStart = [&](const auto& item) -> int64_t {
			if (const auto* mapped = mappedAnimation(item.value))
				return int64_t(mapped->startMs) + std::llround((int64_t(originalSourceMs(item.value)) - mapped->source.sourceStartMs) / mapped->source.playRate);
			return animationBatchBase + item.offsetMs;
		};
		const auto mutate = [&](std::string& message) {
			return m_pBalanceTool->Apply_ValtanCompositionDraftTransaction([&](std::string& diagnostic) {
				if (!animationItems.empty())
				{
					CBalanceTool::PATTERN_STAGE_EDIT candidate;
					if (!m_pBalanceTool->Get_ValtanStageDraft(patternId, stageId, candidate, diagnostic)) return false;
					if (!candidate.animationEditable)
					{ diagnostic = "Animation batch append requires an editable finite destination Sequence."; return false; }
					uint32_t appendStart = 0u;
					if (!candidate.animationSlots.empty() && !ComputeExactAnimationWallMs(candidate, appendStart))
					{ diagnostic = "Finish the destination Animation loop before appending a copied batch."; return false; }
					animationBatchBase = int64_t(appendStart) - animationItems.front()->offsetMs;
					uint64_t cursor = appendStart;
					for (const auto* item : animationItems)
					{
						auto slot = *item->value.animation;
						if (!slot.playMs || slot.repeatUntilStageEnd || !std::isfinite(slot.playRate) || slot.playRate <= 0.0)
						{ diagnostic = "A copied Animation no longer has a finite clock."; return false; }
						slot.clipOccurrenceId = BuildNextCompositionSlotId(patternId, stageId, candidate.animationSlots);
						if (slot.clipOccurrenceId.empty()) { diagnostic = "No free copied Animation occurrence ID."; return false; }
						slot.mappingBasis = "PROJECT_AUTHORED";
						copiedAnimations[sourceKey(item->value, item->value.animation->clipOccurrenceId)] =
							{slot.clipOccurrenceId, static_cast<uint32_t>(cursor), *item->value.animation};
						cursor += static_cast<uint64_t>(std::llround(slot.playMs / slot.playRate));
						if (cursor > 600000u) { diagnostic = "Copied Animation Sequence exceeds 600000 ms."; return false; }
						candidate.animationSlots.push_back(slot);
						nextSelection.push_back({patternId, stageId, slot.clipOccurrenceId, DETAIL_OWNER::ANIMATION});
					}
					uint64_t required = cursor;
					for (const auto& item : batch->items)
					{
						if (item.value.animation) continue;
						const int64_t at = mixedAnimationStart(item);
						if (at < 0 || at >= 600000) { diagnostic = "Copied boxes before the first Animation need room before its destination; the batch was preserved."; return false; }
						uint64_t end = uint64_t(at) + 1u;
						if (item.value.collider)
						{
							uint32_t first = 0u, last = 0u;
							if (!ResolveValtanColliderScheduleEndpoints(*item.value.collider, first, last, diagnostic)) return false;
							end += last - first;
						}
						required = (std::max)(required, end);
					}
					if (required > 600000u) { diagnostic = "Copied batch exceeds the Stage clock bound."; return false; }
					candidate.durationMs = (std::max)(candidate.durationMs, static_cast<uint32_t>(required));
					if (!NormalizeAnimationSlotDraftClock(candidate, true, diagnostic) ||
						!SetValtanStageDraftWithSoundDependencyAdmission(m_pAnimationTool, m_pBalanceTool,
							m_bPatternShakesReady ? &m_PatternShakes : nullptr, pattern, *stage, candidate, diagnostic)) return false;
					for (const auto* item : animationItems)
						for (const auto& source : item->value.animationSources)
							if (!m_pBalanceTool->Set_ValtanStageSequenceDraft(patternId, stageId, source.iSourceActionId,
								source.iSequenceIndex, candidate, diagnostic)) return false;
				}
				for (const auto& item : batch->items)
				{
					if (item.value.animation) continue;
					const uint64_t at = uint64_t(stageStart) + localStart + item.offsetMs;
					uint64_t cursor = 0u;
					VALTAN_PATTERN_VIEW current;
					if (!m_pBalanceTool->Get_ValtanPatternDraft(patternId, current, diagnostic)) return false;
					std::string targetId = stageId;
					if (animationItems.empty())
					{
						const VALTAN_STAGE_VIEW* target = nullptr;
						for (const auto* candidate : path)
						{
							if (at >= cursor && at - cursor < candidate->iDurationMs) { target = candidate; break; }
							cursor += candidate->iDurationMs;
						}
						if (!target) { diagnostic = "The complete copied batch must start inside the destination Pattern timeline."; return false; }
						targetId = target->strStageId;
					}
					const auto targetStage = std::find_if(current.Stages.begin(), current.Stages.end(),
						[&](const auto& candidate) { return candidate.strStageId == targetId; });
					if (targetStage == current.Stages.end()) { diagnostic = "A destination Stage no longer exists."; return false; }
					const uint32_t stageMs = animationItems.empty() ? static_cast<uint32_t>(at - cursor) : static_cast<uint32_t>(mixedAnimationStart(item));
					const VALTAN_CLIP_OCCURRENCE_VIEW* clip = nullptr;
					uint64_t sourceMs = 0u, clipCursor = 0u;
					for (const auto& candidate : targetStage->ClipOccurrences)
					{
						if (stageMs >= clipCursor && stageMs - clipCursor < candidate.iAuthoringWallMs &&
							std::isfinite(candidate.fPlayRate) && candidate.fPlayRate > 0.f)
						{
							clip = &candidate;
							sourceMs = candidate.iSourceStartMs + static_cast<uint64_t>(std::llround((stageMs - clipCursor) * double(candidate.fPlayRate)));
							break;
						}
						clipCursor += candidate.iAuthoringWallMs;
					}
					const auto& value = item.value;
					if (const auto* mapped = mappedAnimation(value))
					{
						const auto found = std::find_if(targetStage->ClipOccurrences.begin(), targetStage->ClipOccurrences.end(),
							[&](const auto& candidate) { return candidate.strClipOccurrenceId == mapped->occurrenceId; });
						if (found == targetStage->ClipOccurrences.end()) { diagnostic = "A copied cue lost its copied Animation identity."; return false; }
						clip = &*found; sourceMs = originalSourceMs(value);
					}
					if (!value.collider && (!value.effect || !value.effect->bUsesStageClock) &&
						(!value.effectV2 || value.effectV2->eClockBasis != EFFECT_V2_CLOCK_BASIS::STAGE) && (!clip || sourceMs > 600000u))
					{ diagnostic = "Every copied clip-owned box needs an Animation at its destination time."; return false; }
					TIMELINE_SELECTION selected{patternId, targetStage->strStageId, {}, value.owner};
					if (value.collider)
					{
						CBalanceTool::PATTERN_STAGE_EDIT candidate;
						if (!m_pBalanceTool->Get_ValtanStageDraft(patternId, targetStage->strStageId, candidate, diagnostic) ||
							!BuildValtanColliderPasteDraft(*value.collider, stageMs, candidate, diagnostic) ||
							!m_pBalanceTool->Set_ValtanStageDraft(patternId, targetStage->strStageId, candidate, diagnostic)) return false;
						selected.strStableId = targetStage->strStageId + "/collider";
					}
					else if (value.effect)
					{
						auto cue = *value.effect;
						cue.iPlaybackOffsetMs = ResolveEffectPlaybackOffsetMs(cue); cue.bHasPlaybackOffset = true;
						cue.strPatternId = patternId; cue.strStageId = targetStage->strStageId; cue.strActionId = targetStage->strActionId;
						cue.strClipOccurrenceId = cue.bUsesStageClock ? std::string{} : clip->strClipOccurrenceId;
						cue.iStageDurationMs = targetStage->iDurationMs;
						const uint64_t effectStartMs = cue.bUsesStageClock ? stageMs : sourceMs;
						const double effectRate = cue.bUsesStageClock ? 1.0 : double(clip->fPlayRate);
						if (cue.bUsesStageClock) cue.iStageOffsetMs = stageMs;
						cue.strBindingId = BuildNextCompositionEffectCueId(current, *targetStage);
						if (cue.strBindingId.empty()) { diagnostic = "No free destination Effect invocation ID."; return false; }
						cue.strOccurrenceId = cue.strBindingId + ".occurrence.01";
						cue.iSourceStartMs = static_cast<uint32_t>(effectStartMs);
						if (cue.bHasSourceEnd)
						{
							const uint64_t endMs = effectStartMs + static_cast<uint64_t>(std::llround(item.effectDurationWallMs * effectRate));
							if (endMs <= effectStartMs || endMs > 600000u) { diagnostic = "A copied Effect end exceeds its destination source clock."; return false; }
							cue.iSourceEndMs = static_cast<uint32_t>(endMs);
						}
						if (!ValidateValtanEffectInvocationClock(m_pAnimationTool, *targetStage, cue, diagnostic) ||
							!m_pBalanceTool->Add_ValtanStageEffectCue(patternId, targetStage->strStageId, targetStage->strActionId, cue, diagnostic)) return false;
						selected.strStableId = cue.strOccurrenceId;
					}
					else if (value.sound)
					{
						const auto& cue = *value.sound;
						VALTAN_PATTERN_SOUND_CUE_ROW_ID created;
						if (!m_pAnimationTool->Add_ValtanCompositionPatternSound(current, *targetStage, clip->strClipOccurrenceId,
							cue.strSoundEvent, static_cast<uint32_t>(sourceMs), cue.eRepeatPolicy, created, diagnostic,
							cue.bHasPlaybackOffset ? std::optional<uint32_t>{cue.iPlaybackOffsetMs} : std::nullopt,
							cue.bHasPlaybackDuration ? std::optional<uint32_t>{cue.iPlaybackDurationMs} : std::nullopt)) return false;
						selected.strStableId = created.strOccurrenceId;
					}
					else if (value.effectV2)
					{
						auto cue = *value.effectV2;
						cue.strPatternId = patternId; cue.strStageId = targetStage->strStageId; cue.strActionId = targetStage->strActionId;
						cue.iStartMs = cue.eClockBasis == EFFECT_V2_CLOCK_BASIS::STAGE ? stageMs : static_cast<uint32_t>(sourceMs);
						if (cue.eClockBasis == EFFECT_V2_CLOCK_BASIS::CLIP_OCCURRENCE) cue.strClipOccurrenceId = clip->strClipOccurrenceId;
						if (!Validate_EffectV2BindingClock(*targetStage, cue, diagnostic)) return false;
						v2SelectionIndexes.push_back(nextSelection.size()); v2Bindings.push_back(std::move(cue));
					}
					else { diagnostic = "The clipboard contains an unsupported group owner."; return false; }
					nextSelection.push_back(std::move(selected));
				}
				// 마지막에 V2 후보 하나를 commit한다. 실패하면 바깥의 Balance·Sound transaction을 rollback해
				// 일부 소유 도구에만 붙여넣기가 남지 않게 한다.
				if (!v2Bindings.empty())
				{
					return CEffectV2Catalog::Get().Stage_AppendBossValtanBindings(v2Bindings, v2Ids, diagnostic);
				}
				return true;
			}, message);
		};
		const bool applied = hasSound ? m_pAnimationTool->Apply_ValtanCompositionPatternSoundDraftTransaction(mutate, status) : mutate(status);
		if (!applied) { status = "Paste rejected; the entire destination draft was preserved. " + status; return false; }
		// 세 데이터 소유 도구의 commit이 모두 끝난 뒤 선택 문자열을 만든다.
		for (std::size_t index = 0u; index < v2Ids.size(); ++index)
		{
			v2Bindings[index].strBindingId = std::move(v2Ids[index]);
			nextSelection[v2SelectionIndexes[index]].strStableId = BuildEffectV2BindingStableId(v2Bindings[index], v2Bindings[index].strClipOccurrenceId);
		}
		m_TimelineSelection = std::move(nextSelection);
		const auto& selected = m_TimelineSelection.back();
		m_strSelectedPatternId = patternId; m_strSelectedStageId = selected.strStageId;
		m_strSelectedStableId = selected.strStableId; m_eDetailOwner = selected.eOwner;
		m_strEffectEditIdentity.clear(); Invalidate_EffectivePatternCache(); Invalidate_TimelineCache();
		m_bAuthoringDraftDirty = m_pBalanceTool->Is_ValtanDraftDirty();
		status = "Pasted " + std::to_string(batch->items.size()) +
			(animationItems.empty() ? " Effect/Sound/Collider boxes with relative timing. " :
			 " Animation/Effect/Sound/Collider boxes. Animation slots append in source order; copied cues retain their copied clip identities and timing. ") +
			"Same-shape Collider pulses share their Stage box. Save commits the existing typed owners.";
		return true;
	}
	const auto clipAt = [&](uint32_t at, uint32_t& sourceMs) -> const VALTAN_CLIP_OCCURRENCE_VIEW* {
		uint32_t cursor = 0u;
		for (const auto& clip : stage->ClipOccurrences)
		{
			if (at >= cursor && at - cursor < clip.iAuthoringWallMs && std::isfinite(clip.fPlayRate) && clip.fPlayRate > 0.f)
			{ sourceMs = clip.iSourceStartMs + static_cast<uint32_t>(std::llround((at - cursor) * double(clip.fPlayRate))); return &clip; }
			cursor += clip.iAuthoringWallMs;
		}
		return nullptr;
	};
	auto native = std::dynamic_pointer_cast<const VALTAN_EDIT_TRANSFER>(transfer);
	if (const auto effects = std::dynamic_pointer_cast<const VALTAN_EFFECT_TRANSFER>(transfer)) native = effects->native;
	const auto finish = [&](bool success, DETAIL_OWNER owner, const std::string& id) {
		if (!success) return false;
		m_strSelectedPatternId = patternId; m_strSelectedStageId = stageId; m_strSelectedStableId = id; m_eDetailOwner = owner;
		m_strEffectEditIdentity.clear(); Invalidate_EffectivePatternCache(); Invalidate_TimelineCache();
		status = "Pasted a new occurrence into the draft. Save commits it through the existing typed owners.";
		return true;
	};
	if (native)
	{
		if (native->collider)
		{
			CBalanceTool::PATTERN_STAGE_EDIT candidate;
			if (!m_pBalanceTool->Get_ValtanStageDraft(patternId, stageId, candidate, status)) return false;
			uint32_t at = localStart;
			if (duplicate && native->patternId == patternId && native->stageId == stageId)
			{
				uint32_t first = 0u, last = 0u;
				if (!ResolveValtanColliderScheduleEndpoints(candidate, first, last, status)) return false;
				at = last + 1u;
			}
			if (!BuildValtanColliderPasteDraft(*native->collider, at, candidate, status)) return false;
			if (!finish(m_pBalanceTool->Set_ValtanStageDraft(patternId, stageId, candidate, status),
				DETAIL_OWNER::GAMEPLAY_STAGE, stageId + "/collider")) return false;
			status = "Pasted Collider geometry and timing. Same-shape pulses share the Stage Collider box; Save and Publish apply the Server-owned hit schedule.";
			return true;
		}
		if (native->animation)
		{
			CBalanceTool::PATTERN_STAGE_EDIT draft;
			if (!m_pBalanceTool->Get_ValtanStageDraft(patternId, stageId, draft, status)) return false;
			auto row = *native->animation;
			row.clipOccurrenceId = BuildNextCompositionSlotId(patternId, stageId, draft.animationSlots);
			row.mappingBasis = "PROJECT_AUTHORED";
			auto at = std::find_if(draft.animationSlots.begin(), draft.animationSlots.end(), [&](const auto& x) { return x.clipOccurrenceId == m_strSelectedStableId; });
			if (at != draft.animationSlots.end()) ++at;
			draft.animationSlots.insert(at, row);
			if (!NormalizeAnimationSlotDraftClock(draft, true, status)) return false;
			const bool applied = m_pBalanceTool->Apply_ValtanCompositionDraftTransaction([&](std::string& message) {
				if (!SetValtanStageDraftWithSoundDependencyAdmission(m_pAnimationTool, m_pBalanceTool,
					m_bPatternShakesReady ? &m_PatternShakes : nullptr, pattern, *stage, draft, message)) return false;
				for (const auto& source : native->animationSources)
					if (!m_pBalanceTool->Set_ValtanStageSequenceDraft(patternId, stageId, source.iSourceActionId,
						source.iSequenceIndex, draft, message)) return false;
				return true;
			}, status);
			return finish(applied, DETAIL_OWNER::ANIMATION, row.clipOccurrenceId);
		}
		if (native->effect)
		{
			auto cue = *native->effect;
			cue.iPlaybackOffsetMs = ResolveEffectPlaybackOffsetMs(cue);
			cue.bHasPlaybackOffset = true;
			if (!duplicate || native->stageId != stageId || native->patternId != patternId)
			{
				uint32_t source = localStart;
				const uint32_t sourceLength = cue.bHasSourceEnd ? cue.iSourceEndMs - cue.iSourceStartMs : 0u;
				if (cue.bUsesStageClock)
				{
					cue.strClipOccurrenceId.clear(); cue.iStageOffsetMs = localStart;
				}
				else
				{
					const auto* clip = clipAt(localStart, source);
					if (!clip) { status = "Place the playhead inside a destination Animation occurrence."; return false; }
					cue.strClipOccurrenceId = clip->strClipOccurrenceId;
				}
				cue.iSourceStartMs = source;
				cue.iSourceEndMs = cue.bHasSourceEnd ? source + sourceLength : 0u;
			}
			cue.strPatternId = patternId; cue.strStageId = stageId; cue.strActionId = stage->strActionId;
			cue.iStageDurationMs = stage->iDurationMs; cue.strBindingId = BuildNextCompositionEffectCueId(pattern, *stage);
			cue.strOccurrenceId = cue.strBindingId + ".occurrence.01";
			return finish(ValidateValtanEffectInvocationClock(m_pAnimationTool, *stage, cue, status) &&
				m_pBalanceTool->Add_ValtanStageEffectCue(patternId, stageId, stage->strActionId, cue, status), DETAIL_OWNER::EFFECT, cue.strOccurrenceId);
		}
		if (native->effectV2)
		{
			auto cue = *native->effectV2;
			if (!duplicate || native->stageId != stageId || native->patternId != patternId)
			{
				if (cue.eClockBasis == EFFECT_V2_CLOCK_BASIS::STAGE) cue.iStartMs = localStart;
				else { uint32_t source = 0u; const auto* clip = clipAt(localStart, source);
					if (!clip) { status = "Place the playhead inside a destination Animation occurrence."; return false; }
					cue.strClipOccurrenceId = clip->strClipOccurrenceId; cue.iStartMs = source; }
			}
			cue.strPatternId = patternId; cue.strStageId = stageId; cue.strActionId = stage->strActionId;
			if (!Validate_EffectV2BindingClock(*stage, cue, status)) return false;
			std::vector<std::string> ids;
			if (!CEffectV2Catalog::Get().Stage_AppendBossValtanBindings({cue}, ids, status)) return false;
			cue.strBindingId = ids.front();
			return finish(true, DETAIL_OWNER::EFFECT, BuildEffectV2BindingStableId(cue, cue.strClipOccurrenceId));
		}
		if (native->sound)
		{
			auto cue = *native->sound;
			uint32_t source = cue.iStartMs; std::string clipId = cue.strClipOccurrenceId;
			if (!duplicate || native->stageId != stageId || native->patternId != patternId)
			{ const auto* clip = clipAt(localStart, source); if (!clip) { status = "Sound paste needs an Animation at the playhead."; return false; } clipId = clip->strClipOccurrenceId; }
			VALTAN_PATTERN_SOUND_CUE_ROW_ID created;
			const bool applied = m_pAnimationTool && m_pAnimationTool->Add_ValtanCompositionPatternSound(pattern, *stage, clipId,
				cue.strSoundEvent, source, cue.eRepeatPolicy, created, status,
				cue.bHasPlaybackOffset ? std::optional<uint32_t>{cue.iPlaybackOffsetMs} : std::nullopt,
				cue.bHasPlaybackDuration ? std::optional<uint32_t>{cue.iPlaybackDurationMs} : std::nullopt);
			return finish(applied, DETAIL_OWNER::SOUND, created.strOccurrenceId);
		}
		if (native->scene)
		{
			auto rows = stage->SceneProfileOccurrences; auto row = *native->scene;
			row.strOccurrenceId = NextValtanOccurrenceId(patternId + "." + stageId + ".scene.copy.", rows, [](const auto& x) { return x.strOccurrenceId; });
			if (!duplicate) row.iStartMs = localStart;
			rows.push_back(row);
			return finish(m_pBalanceTool->Set_ValtanStageSceneProfileOccurrences(patternId, stageId, rows, status), DETAIL_OWNER::SCENE_PROFILE, row.strOccurrenceId);
		}
		if (native->light)
		{
			auto rows = stage->LightOccurrences; auto row = *native->light;
			row.strOccurrenceId = NextValtanOccurrenceId(patternId + "." + stageId + ".light.copy.", rows, [](const auto& x) { return x.strOccurrenceId; });
			if (!duplicate) row.iStartMs = localStart;
			rows.push_back(row);
			return finish(m_pBalanceTool->Set_ValtanStageLightOccurrences(patternId, stageId, rows, status), DETAIL_OWNER::LIGHT, row.strOccurrenceId);
		}
		if (native->camera)
		{
			auto row = *native->camera;
			row.strCameraInvocationId = NextValtanOccurrenceId(patternId + "." + stageId + ".camera.copy.", stage->CameraInvocations, [](const auto& x) { return x.strCameraInvocationId; });
			if (!duplicate) row.iStartOffsetMs = localStart;
			return finish(m_pBalanceTool->Upsert_ValtanCameraInvocationDraft(patternId, stageId, row, status), DETAIL_OWNER::CAMERA, row.strCameraInvocationId);
		}
		if (!native->actions.empty())
		{
			for (const auto& action : native->actions)
				if (std::any_of(stage->Actions.begin(), stage->Actions.end(), [&](const auto& x) { return x.strKind == action.strKind && x.strTrigger == action.strTrigger && x.strTargetId == action.strTargetId; }))
				{ status = "The destination already owns this Logic action; Paste does not overwrite it."; return false; }
			const bool applied = m_pBalanceTool->Apply_ValtanCompositionDraftTransaction([&](std::string& message) {
				for (const auto& action : native->actions) if (!m_pBalanceTool->Upsert_ValtanStageActionDraft(patternId, stageId, action, message)) return false;
				return true;
			}, status);
			const auto& first = native->actions.front();
			return finish(applied, DETAIL_OWNER::GAMEPLAY_STAGE, stageId + "/action/" + first.strKind + "/" + first.strTargetId + "/" + first.strTrigger);
		}
		if (native->summon)
		{
			VALTAN_COMBAT_OBJECT_EFFECT_VIEW created;
			const bool applied = m_pBalanceTool->Clone_ValtanSummonDraft(patternId, stageId, *native->summon,
				native->sourceRevision, duplicate ? native->summon->iFirstSpawnOffsetMs : localStart, created, status);
			return finish(applied, DETAIL_OWNER::COMBAT_OBJECT, created.strCombatObjectArchetypeId);
		}
	}
	const auto effects = std::dynamic_pointer_cast<const COMPOSITION_EFFECT_TRANSFER>(transfer);
	if (!effects || effects->items.empty() || effects->items.size() > 128u)
	{ status = "This resource has no supported Valtan transfer adapter."; return false; }
	const bool v1 = effects->items.front().resourceKind == "V1_EFFECT";
	std::vector<VALTAN_PRODUCT_EFFECT_CUE_VIEW> cues;
	std::vector<EFFECT_V2_BINDING> bindings;
	for (const auto& item : effects->items)
	{
		if (!item.followOwner)
		{ status = "Fixed/world snapshot anchors require their original owner; paste was preserved."; return false; }
		if (item.resourceId.empty() || !item.bone.empty() || item.fitToDuration || item.loopToDuration ||
			(item.resourceKind != "V1_EFFECT" && item.resourceKind != "LEAF" && item.resourceKind != "GROUP") ||
			v1 != (item.resourceKind == "V1_EFFECT"))
		{ status = "Paste accepts one homogeneous V1 or V2 root Effect batch; bone, fit, loop and mixed owners need their typed editors."; return false; }
		const uint64_t at = uint64_t(localStart) + item.startMs;
		if (at >= stage->iDurationMs || at + item.durationMs > 600000u)
		{ status = "Each Effect must start inside the destination Stage and end within 600000 ms."; return false; }
		if (v1)
		{
			if (!item.inheritOwnerRotation) { status = "Valtan V1 cue rotation follows its root; world rotation cannot be discarded."; return false; }
			VALTAN_PRODUCT_EFFECT_CUE_VIEW cue;
			cue.bUsesStageClock = true; cue.iStageOffsetMs = static_cast<uint32_t>(at);
			cue.strPatternId = patternId; cue.strStageId = stageId; cue.strActionId = stage->strActionId;
			cue.strBindingId = BuildNextCompositionEffectCueId(pattern, *stage); cue.strOccurrenceId = cue.strBindingId + ".occurrence.01";
			cue.strEffectAssetId = item.resourceId; cue.strAnchorSlotId = "root";
			cue.eFollowPolicy = item.followOwner ? EFFECT_FOLLOW_POLICY::FOLLOW : EFFECT_FOLLOW_POLICY::SNAPSHOT;
			cue.strFollowPolicy = item.followOwner ? "follow" : "snapshot"; cue.strRepeatPolicy = "once";
			cue.eScalePolicy = VALTAN_PATTERN_EFFECT_SCALE_POLICY::OWNER_RELATIVE; cue.strScalePolicy = "OWNER_RELATIVE"; cue.bHasExplicitScalePolicy = true;
			cue.bHasSourceEnd = item.durationMs != 0u; cue.iSourceStartMs = cue.iStageOffsetMs;
			cue.iSourceEndMs = cue.bHasSourceEnd ? cue.iStageOffsetMs + item.durationMs : 0u;
			cue.eStopPolicy = cue.bHasSourceEnd ? EFFECT_STOP_POLICY::CUE_END : EFFECT_STOP_POLICY::NATURAL;
			cue.strStopPolicy = cue.bHasSourceEnd ? "cue_end" : "natural"; cue.iStageDurationMs = stage->iDurationMs;
			cue.LocalTransform.vPosition = {item.position[0], item.position[1], item.position[2]};
			cue.LocalTransform.vRotationDegrees = {item.rotationDegrees[0], item.rotationDegrees[1], item.rotationDegrees[2]};
			cue.LocalTransform.vScale = {item.scale[0], item.scale[1], item.scale[2]};
			stage->ProductCues.push_back(cue); cues.push_back(std::move(cue));
		}
		else
		{
			if (item.durationMs) { status = "Valtan V2 bindings use natural/Stage/clip stop policies; an explicit copied duration cannot be discarded."; return false; }
			EFFECT_V2_BINDING cue;
			cue.eResourceKind = item.resourceKind == "GROUP" ? EFFECT_V2_RESOURCE_KIND::GROUP : EFFECT_V2_RESOURCE_KIND::LEAF;
			cue.strResourceId = item.resourceId; cue.strPatternId = patternId; cue.strStageId = stageId; cue.strActionId = stage->strActionId;
			cue.eClockBasis = EFFECT_V2_CLOCK_BASIS::STAGE; cue.iStartMs = static_cast<uint32_t>(at); cue.strAnchorSlotId = "b_effectroot";
			cue.eFollowPolicy = item.followOwner ? EFFECT_V2_FOLLOW_POLICY::FOLLOW_SLOT : EFFECT_V2_FOLLOW_POLICY::SNAPSHOT_AT_START;
			cue.eRotationBasis = item.inheritOwnerRotation ? EFFECT_V2_ROTATION_BASIS::TARGET_YAW : EFFECT_V2_ROTATION_BASIS::WORLD;
			cue.eStopPolicy = EFFECT_V2_STOP_POLICY::NATURAL;
			cue.LocalTransform.vTranslation = {item.position[0], item.position[1], item.position[2]};
			cue.LocalTransform.vRotation = {item.rotationDegrees[0], item.rotationDegrees[1], item.rotationDegrees[2]};
			cue.LocalTransform.vScale = {item.scale[0], item.scale[1], item.scale[2]}; bindings.push_back(cue);
		}
	}
	if (v1)
	{
		const bool applied = m_pBalanceTool->Apply_ValtanCompositionDraftTransaction([&](std::string& message) {
			for (const auto& cue : cues) if (!m_pBalanceTool->Add_ValtanStageEffectCue(patternId, stageId, stage->strActionId, cue, message)) return false;
			return true;
		}, status);
		return finish(applied, DETAIL_OWNER::EFFECT, cues.back().strOccurrenceId);
	}
	std::vector<std::string> ids;
	if (!CEffectV2Catalog::Get().Stage_AppendBossValtanBindings(bindings, ids, status)) return false;
	bindings.back().strBindingId = ids.back();
	return finish(true, DETAIL_OWNER::EFFECT, BuildEffectV2BindingStableId(bindings.back()));
}

bool Client::CValtanActionWorkbench::Can_AppendCompositionAnimationResource(
	const COMPOSITION_ANIMATION_RESOURCE& resource, const bool asNewStage,
	std::string& status) const
{
	if (resource.strTargetAssetName != "Valtan")
	{
		status = "Valtan Patterns store clips for the Valtan body. Other models remain available for Preview.";
		return false;
	}
	const auto asset = std::find_if(ANIMATION_PREVIEW_ASSETS.begin(), ANIMATION_PREVIEW_ASSETS.end(),
		[](const auto& candidate) { return std::string_view("Valtan") == candidate.pAssetName; });
	if (asset == ANIMATION_PREVIEW_ASSETS.end() || resource.strModelAssetId != asset->pModelAssetId ||
		(resource.strSourceAssetId != asset->pModelAssetId &&
		 (nullptr == asset->pAnimationSetAssetId || resource.strSourceAssetId != asset->pAnimationSetAssetId) &&
         !(resource.strSourceAssetId == "Data/Animation/Authored/Valtan/Valtan.boneclips.json" && resource.strRuntimeClip.starts_with("authored."))))
	{
		status = "The physical clip does not belong to the Valtan body or its animation package.";
		return false;
	}
	if (m_PendingResourceAppend.has_value())
	{
		status = "An animation append is already queued for the end of this frame.";
		return false;
	}
	if (nullptr == m_pBalanceTool)
	{
		status = "The Valtan source document owner is unavailable.";
		return false;
	}
	if (!Can_MutateValtanView(m_eAdmission) ||
		(m_pAnimationTool && m_pAnimationTool->Is_ValtanCompositionPatternTransactionActive()))
	{
		status = "Finish Create Pattern and reload the admitted source before appending an Animation.";
		return false;
	}
	std::string authoringRevision, sourceRevision, revisionStatus;
	bool dirty = false;
	if (!m_pBalanceTool->Get_ValtanAuthoringState(authoringRevision, dirty, revisionStatus) ||
		!m_pBalanceTool->Get_ValtanCanonicalSourceRevision(sourceRevision, revisionStatus) ||
		authoringRevision != m_strPinnedAuthoringSourceRevision ||
		sourceRevision != m_strPinnedCanonicalSourceRevision)
	{
		status = "The selected Pattern and Balance draft belong to different source revisions. Reload the Valtan session before appending. " + revisionStatus;
		return false;
	}
	// resource 추가는 Pattern 끝을 대상으로 하며 canonical gameplay의 Stage graph를 유지한다.
	(void)asNewStage;
	VALTAN_PATTERN_VIEW pattern;
	if (!m_pBalanceTool->Get_ValtanPatternDraft(m_strSelectedPatternId, pattern, status) || pattern.Stages.empty())
		return false;
	return m_pBalanceTool->Can_AppendValtanAnimationClip(pattern.strPatternId,
		pattern.Stages.back().strStageId, resource.strRuntimeClip, resource.iDurationMs, pattern.bManualServerAudition, status);
}

bool Client::CValtanActionWorkbench::Append_CompositionAnimationResource(
	const COMPOSITION_ANIMATION_RESOURCE& resource, const bool asNewStage,
	std::string& status)
{
	if (!Can_AppendCompositionAnimationResource(resource, asNewStage, status))
		return false;
	PENDING_RESOURCE_APPEND command{ resource, true, m_strSelectedPatternId, {} };
	if (m_bWorkbenchFrameActive)
	{
		m_PendingResourceAppend = std::move(command);
		status = "Animation append queued; the draft updates after the current panes finish.";
		return true;
	}
	return Apply_CompositionResourceAppend(command, status);
}

bool Client::CValtanActionWorkbench::Apply_CompositionResourceAppend(
	const PENDING_RESOURCE_APPEND& command, std::string& status)
{
	HISTORY_SCOPE history(*this);
	if (command.strPatternId != m_strSelectedPatternId)
	{ status = "The selected Pattern changed before append; the queued append was canceled without changing a draft."; return false; }
	if (!Can_AppendCompositionAnimationResource(command.Resource, true, status))
		return false;
	VALTAN_PATTERN_VIEW pattern;
	if (!m_pBalanceTool->Get_ValtanPatternDraft(command.strPatternId, pattern, status) || pattern.Stages.empty())
		return false;
	const auto endStageId = pattern.Stages.back().strStageId;
	const bool asNewStage = pattern.bManualServerAudition;
	std::string stageId;
	std::string occurrenceId;
	if (!m_pBalanceTool->Append_ValtanAnimationClip(
		command.strPatternId, endStageId, command.Resource.strRuntimeClip,
		command.Resource.iDurationMs, asNewStage, pattern, stageId, occurrenceId, status))
		return false;
	m_EffectivePatternCache = std::move(pattern);
	m_strEffectivePatternCachePatternId = command.strPatternId;
	m_strEffectivePatternCacheCanonicalRevision = m_strPinnedCanonicalSourceRevision;
	m_iEffectivePatternCacheDraftGeneration = m_pBalanceTool->Get_ValtanDraftGeneration();
	m_bEffectivePatternCacheReady = true;
	m_bCompositionResourceDraftReady = true;
	m_bAuthoringDraftDirty = true;
	m_strSelectedPatternId = command.strPatternId;
	m_strSelectedStageId = stageId;
	m_strSelectedStableId = occurrenceId;
	m_eDetailOwner = DETAIL_OWNER::ANIMATION;
	m_TimelineSelection.clear();
	if (asNewStage) m_TimelineSelection.push_back({command.strPatternId, stageId, stageId, DETAIL_OWNER::GAMEPLAY_STAGE});
	m_TimelineSelection.push_back({command.strPatternId, stageId, occurrenceId, DETAIL_OWNER::ANIMATION});
	m_BossPatternOutcomeOverrides.clear(); ++m_iBossPatternRouteGeneration;
	m_strEffectEditIdentity.clear();
	Invalidate_TimelineCache();
	status = asNewStage ? "Appended Animation at the Pattern end. Earlier/Later moves its selected Stage and clip together. Save when ready." :
		"Appended Animation after the final clip in the existing final Stage. Gameplay topology is preserved. Save when ready.";
	Refresh_PatternLocalPreviewAfterMutation(&m_EffectivePatternCache, status);
	m_strStatus = status;
	return true;
}

// 모든 pane 호출 뒤 frame 포인터를 해제하고 대기 Save·Reload·선택 요청을 처리한다.
// 이 경계를 두어 한 창의 저장이 다른 창이 아직 읽는 vector를 교체하지 않게 한다.
void Client::CValtanActionWorkbench::End_WorkbenchFrame()
{
	if (!m_bWorkbenchFrameActive)
		return;
	// Save나 Reload가 canonical 저장소를 교체할 때는 어떤 pane도 그 안의 포인터를 보관하면 안 된다.
	m_bWorkbenchFrameActive = false;
	m_pWorkbenchFramePattern = nullptr;
	m_pWorkbenchFrameStage = nullptr;
	if (m_PendingCompositionPaste)
	{
		const auto pending = std::move(*m_PendingCompositionPaste); m_PendingCompositionPaste.reset();
		(void)Apply_CompositionTransfer(pending.transfer, pending.patternId, pending.stageId, pending.playheadMs, pending.duplicate, m_strStatus);
	}
	if (m_PendingResourceAppend.has_value())
	{
		const PENDING_RESOURCE_APPEND command = std::move(*m_PendingResourceAppend);
		m_PendingResourceAppend.reset();
		(void)Apply_CompositionResourceAppend(command, m_strStatus);
	}
	Process_EditHistoryRequest();
	if (m_bSavePatternRequested)
	{
		/* Details는 이 명령을 대기열에 넣기만 한다. 모든 창이 현재 프레임의 불변 Pattern/Stage view 사용을 마친 뒤
		   성공한 Save의 재로드가 canonical 저장소를 교체한다. */
		m_bSavePatternRequested = false;
		const bool_t bSaveStarted = Save_Reload();
		m_bPatternSaveSucceeded = false;
		m_bPatternSaveResultAvailable = !bSaveStarted;
		m_strPatternSaveStatus = m_strStatus;
	}
	/* Pattern Browser의 요청에는 stable ID만 남긴다. 모든 창과 일반 지연 Save가
	   현재 프레임의 불변 Pattern/Stage 포인터 사용을 마친 뒤 그 ID를 다시 조회한다. */
	Resolve_PendingPatternSelection();
	m_bApplyResetLayoutThisFrame = false;
	if (m_bWorkbenchReloadRequested)
	{
		m_bWorkbenchReloadRequested = false;
		(void)Reload_Canonical();
	}
}

void Client::CValtanActionWorkbench::Render()
{
	Begin_WorkbenchFrame();
	const VALTAN_PATTERN_VIEW* const pPattern = m_pWorkbenchFramePattern;
	const VALTAN_STAGE_VIEW* const pStage = m_pWorkbenchFrameStage;
	const std::uint64_t iPatternViewDraftGeneration = m_iWorkbenchFrameDraftGeneration;
	const bool_t bLocalPreviewAdmitted = m_bWorkbenchFrameLocalPreviewAdmitted;
	const bool_t bMutationAdmitted = m_bWorkbenchFrameMutationAdmitted;
	const bool_t bPatternMutationAdmitted = m_bWorkbenchFramePatternMutationAdmitted;
	/* MainApp의 도구 focus 요청은 첫 번째 최상위 Composition 창이 소비한다.
	   주 편집 화면인 Sequencer를 먼저 열며 모든 창은 현재 프레임의 불변 Pattern view를 공유한다. */
	Render_SequencerWindow(
		pPattern, bLocalPreviewAdmitted,
		bMutationAdmitted, bPatternMutationAdmitted);
	if (!m_bTimelineMaximized)
	{
		Render_BossPatternWindow(
			pPattern, iPatternViewDraftGeneration,
			bMutationAdmitted, bPatternMutationAdmitted);
		Render_PatternsWindow(
			pPattern, pStage, bPatternMutationAdmitted);
		Render_PreviewWindow(
			pPattern, bLocalPreviewAdmitted, bPatternMutationAdmitted);
		Render_DetailsWindow(
			pPattern, pStage, bMutationAdmitted, bPatternMutationAdmitted);
		Render_ResourcesWindow(
			pPattern, pStage, bMutationAdmitted, bPatternMutationAdmitted);
	}
	End_WorkbenchFrame();
}

bool_t Client::CValtanActionWorkbench::Consume_EffectResourcePreviewRequest(EFFECT_RESOURCE_KEY& OutKey)
{
	if (!m_PendingEffectResourcePreview) return false;
	OutKey = std::move(*m_PendingEffectResourcePreview);
	m_PendingEffectResourcePreview.reset();
	return true;
}

bool_t Client::CValtanActionWorkbench::Consume_EffectResourceOpenRequest(EFFECT_RESOURCE_KEY& OutKey)
{
	if (!m_PendingEffectResourceOpen) return false;
	OutKey = std::move(*m_PendingEffectResourceOpen);
	m_PendingEffectResourceOpen.reset();
	return true;
}

bool_t Client::CValtanActionWorkbench::Consume_EffectToolOpenRequest(
	EFFECT_TOOL_VALTAN_PRODUCT_OPEN_REQUEST& OutRequest)
{
	if (!m_bEffectToolOpenRequested)
		return false;
	OutRequest.strPatternId = std::move(m_strEffectPatternId);
	OutRequest.strStageId = std::move(m_strEffectStageId);
	OutRequest.strCueOccurrenceId = std::move(m_strEffectOccurrenceId);
	OutRequest.strEffectAssetId = std::move(m_strEffectAssetId);
	m_bEffectToolOpenRequested = false;
	return true;
}

bool_t Client::CValtanActionWorkbench::Consume_CameraToolOpenRequest(
	CAMERA_TOOL_OPEN_REQUEST& OutRequest)
{
	if (!m_bCameraToolOpenRequested)
		return false;
	OutRequest.strCueId = std::move(m_strCameraCueId);
	m_bCameraToolOpenRequested = false;
	return true;
}

bool_t Client::CValtanActionWorkbench::Consume_AnimationToolOpenRequest()
{
	const bool_t bRequested = m_bOpenAnimationToolRequested;
	m_bOpenAnimationToolRequested = false;
	return bRequested;
}
