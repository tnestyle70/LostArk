#pragma once
#include "MapTool.h"
#include "DirectXTK/PrimitiveBatch.h"
#include "DirectXTK/VertexTypes.h"
#include "DirectXTK/Effects.h"


struct Client::CMapTool::NAVIGATION_RENDER_RESOURCES final
{
	shared_ptr<PrimitiveBatch<VertexPositionColor>> pBatch;
	shared_ptr<BasicEffect> pEffect;
	ComPtr<ID3D11InputLayout> pInputLayout;
};