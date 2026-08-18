#include "imgui.h"

#include "HitAreaWire.h"

#include "GameInstance.h"

#include <cmath>

namespace
{
	constexpr f32_t UNITS_TO_METERS = 0.01f;
	constexpr int32_t ARC_SEGMENTS = 48;
}

void Client::CHitAreaWire::Draw(const float4x4_t& Root, const HIT_AREA_SHAPE& Shape, uint32_t iColorRgba)
{
	if (Shape.iAreaType <= 0)
		return;
	const ImU32 iColor = iColorRgba;

	auto& GameInstance = CGameInstance::Get();
	const matrix_t View = XMLoadFloat4x4(GameInstance.Get_Transform(D3DTS::VIEW));
	const matrix_t Proj = XMLoadFloat4x4(GameInstance.Get_Transform(D3DTS::PROJ));
	ImGuiViewport* pViewport = ImGui::GetMainViewport();
	ImDrawList* pDrawList = ImGui::GetBackgroundDrawList(pViewport);

	const matrix_t WorldRoot = XMLoadFloat4x4(&Root);
	const vector_t vPosition = WorldRoot.r[3];
	const vector_t vLook = XMVector3Normalize(XMVectorSetY(WorldRoot.r[2], 0.f));
	const vector_t vRight = XMVector3Normalize(XMVector3Cross(
		XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook));
	const f32_t fGroundY = XMVectorGetY(vPosition) + 0.03f;

	auto Project = [&](fvector_t vWorld, ImVec2& vOut) -> bool_t
	{
		const vector_t vView = XMVector3TransformCoord(vWorld, View);
		if (XMVectorGetZ(vView) < 0.1f)
			return false;
		const vector_t vClip = XMVector3TransformCoord(vView, Proj);
		vOut.x = pViewport->Pos.x + (XMVectorGetX(vClip) * 0.5f + 0.5f) * pViewport->Size.x;
		vOut.y = pViewport->Pos.y + (0.5f - XMVectorGetY(vClip) * 0.5f) * pViewport->Size.y;
		return true;
	};
	auto Draw_Segment = [&](fvector_t vFrom, fvector_t vTo)
	{
		ImVec2 vA{}, vB{};
		if (Project(vFrom, vA) && Project(vTo, vB))
			pDrawList->AddLine(vA, vB, iColor, 2.f);
	};
	auto At = [&](f32_t fCenterForward, f32_t fRadius, f32_t fDegrees)
	{
		const f32_t fRadians = XMConvertToRadians(fDegrees);
		return XMVectorSetY(
			vPosition + vLook * (fCenterForward + fRadius * cosf(fRadians)) +
			vRight * (fRadius * sinf(fRadians)),
			fGroundY);
	};
	auto Draw_Arc = [&](f32_t fCenterForward, f32_t fRadius, f32_t fFromDeg, f32_t fToDeg)
	{
		for (int32_t s = 0; s < ARC_SEGMENTS; ++s)
		{
			const f32_t fA = fFromDeg + (fToDeg - fFromDeg) * s / ARC_SEGMENTS;
			const f32_t fB = fFromDeg + (fToDeg - fFromDeg) * (s + 1) / ARC_SEGMENTS;
			Draw_Segment(At(fCenterForward, fRadius, fA), At(fCenterForward, fRadius, fB));
		}
	};

	const f32_t fOffset = Shape.iAreaOffsetX * UNITS_TO_METERS;
	const f32_t fRange = Shape.iAreaRange * UNITS_TO_METERS;
	/* Official AreaType: 1 circle/ring, 2 forward box whose AreaAngle is the
	width in cm, 3 fan whose AreaAngle is the sweep in degrees. */
	switch (Shape.iAreaType)
	{
	case 2:
	{
		const f32_t fHalfWidth = Shape.iAreaAngle * UNITS_TO_METERS * 0.5f;
		const vector_t vNearL = XMVectorSetY(
			vPosition + vLook * fOffset - vRight * fHalfWidth, fGroundY);
		const vector_t vNearR = XMVectorSetY(
			vPosition + vLook * fOffset + vRight * fHalfWidth, fGroundY);
		const vector_t vFarL = XMVectorSetY(
			vPosition + vLook * (fOffset + fRange) - vRight * fHalfWidth, fGroundY);
		const vector_t vFarR = XMVectorSetY(
			vPosition + vLook * (fOffset + fRange) + vRight * fHalfWidth, fGroundY);
		Draw_Segment(vNearL, vNearR);
		Draw_Segment(vNearR, vFarR);
		Draw_Segment(vFarR, vFarL);
		Draw_Segment(vFarL, vNearL);
		break;
	}
	case 1:
	case 3:
	{
		const bool_t bFullSweep = 1 == Shape.iAreaType ||
			Shape.iAreaAngle <= 0 || Shape.iAreaAngle >= 360;
		const f32_t fHalfSweep = bFullSweep ? 180.f : Shape.iAreaAngle * 0.5f;
		const f32_t fInner = Shape.iAreaInner * UNITS_TO_METERS;
		Draw_Arc(fOffset, fRange, -fHalfSweep, fHalfSweep);
		if (fInner > 0.f)
			Draw_Arc(fOffset, fInner, -fHalfSweep, fHalfSweep);
		if (!bFullSweep)
		{
			Draw_Segment(At(fOffset, fInner, -fHalfSweep), At(fOffset, fRange, -fHalfSweep));
			Draw_Segment(At(fOffset, fInner, fHalfSweep), At(fOffset, fRange, fHalfSweep));
		}
		break;
	}
	default:
		break;
	}
}
