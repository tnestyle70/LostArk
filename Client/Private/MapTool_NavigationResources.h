#pragma once
#include "MapTool.h"
#pragma push_macro("new")
#undef new
#include "DirectXTK/PrimitiveBatch.h"
#include "DirectXTK/VertexTypes.h"
#include "DirectXTK/Effects.h"
#pragma pop_macro("new")


struct Client::CMapTool::NAVIGATION_RENDER_RESOURCES final
{
	shared_ptr<PrimitiveBatch<VertexPositionColor>> pBatch;
	shared_ptr<BasicEffect> pEffect;
	ComPtr<ID3D11InputLayout> pInputLayout;
};