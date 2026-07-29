#include "BinaryAsset/CookedModel.h"

#include "BinaryAsset/ModelDecoderRegistry.h"
#include "Shader.h"

#include <cmath>
#include <cwctype>

namespace
{
	HRESULT CreateSolidTexture(ID3D11Device* pDevice,
		uint32_t rgba,
		ComPtr<ID3D11ShaderResourceView>& outSRV)
	{
		D3D11_TEXTURE2D_DESC textureDesc{};
		textureDesc.Width = 1;
		textureDesc.Height = 1;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA initialData{};
		initialData.pSysMem = &rgba;
		initialData.SysMemPitch = sizeof(rgba);

		ComPtr<ID3D11Texture2D> texture;
		if (FAILED(pDevice->CreateTexture2D(&textureDesc, &initialData, &texture)))
			return E_FAIL;
		return pDevice->CreateShaderResourceView(texture.Get(), nullptr, &outSRV);
	}

	HRESULT LoadTexture(ID3D11Device* pDevice,
		const filesystem::path& path,
		uint32_t fallbackColor,
		ComPtr<ID3D11ShaderResourceView>& outSRV)
	{
		HRESULT hr = E_FAIL;
		if (!path.empty() && filesystem::exists(path))
		{
			wstring extension = path.extension().wstring();
			transform(extension.begin(), extension.end(), extension.begin(),
				[](wchar_t value) { return static_cast<wchar_t>(towlower(value)); });

			if (L".dds" == extension)
				hr = DirectX::CreateDDSTextureFromFile(pDevice, path.c_str(), nullptr, &outSRV);
			else
				hr = DirectX::CreateWICTextureFromFile(pDevice, path.c_str(), nullptr, &outSRV);
		}

		if (FAILED(hr))
			hr = CreateSolidTexture(pDevice, fallbackColor, outSRV);
		return hr;
	}

	template<typename T>
	uint32_t FindKeySegment(const vector<T>& keys, f32_t time)
	{
		if (keys.size() <= 1 || time <= keys.front().timeTicks)
			return 0;
		if (time >= keys.back().timeTicks)
			return static_cast<uint32_t>(keys.size() - 2);

		uint32_t low = 1;
		uint32_t high = static_cast<uint32_t>(keys.size() - 1);
		while (low < high)
		{
			const uint32_t middle = low + ((high - low) >> 1);
			if (keys[middle].timeTicks <= time)
				low = middle + 1;
			else
				high = middle;
		}
		return low - 1;
	}

	vector_t SampleVector(const vector<MODEL_VECTOR_KEY_DATA>& keys,
		f32_t time,
		vector_t fallback)
	{
		if (keys.empty())
			return fallback;
		if (1 == keys.size() || time <= keys.front().timeTicks)
			return XMLoadFloat3(&keys.front().value);
		if (time >= keys.back().timeTicks)
			return XMLoadFloat3(&keys.back().value);

		const uint32_t index = FindKeySegment(keys, time);
		const f32_t span = keys[index + 1].timeTicks - keys[index].timeTicks;
		const f32_t ratio = span > 0.f
			? (clamp)((time - keys[index].timeTicks) / span, 0.f, 1.f)
			: 0.f;
		return XMVectorLerp(
			XMLoadFloat3(&keys[index].value),
			XMLoadFloat3(&keys[index + 1].value),
			ratio);
	}

	vector_t SampleQuaternion(const vector<MODEL_QUAT_KEY_DATA>& keys,
		f32_t time,
		vector_t fallback)
	{
		if (keys.empty())
			return fallback;
		if (1 == keys.size() || time <= keys.front().timeTicks)
			return XMLoadFloat4(&keys.front().value);
		if (time >= keys.back().timeTicks)
			return XMLoadFloat4(&keys.back().value);

		const uint32_t index = FindKeySegment(keys, time);
		const f32_t span = keys[index + 1].timeTicks - keys[index].timeTicks;
		const f32_t ratio = span > 0.f
			? (clamp)((time - keys[index].timeTicks) / span, 0.f, 1.f)
			: 0.f;
		return XMQuaternionNormalize(XMQuaternionSlerp(
			XMLoadFloat4(&keys[index].value),
			XMLoadFloat4(&keys[index + 1].value),
			ratio));
	}
}

CCookedModel::CCookedModel(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: m_pDevice { pDevice }
	, m_pContext { pContext }
{
}

HRESULT CCookedModel::Initialize(const MODEL_ASSET_LOAD_DESC& desc)
{
	MODEL_ASSET_DATA assetData{};
	if (!CModelDecoderRegistry::Get().Decode(desc, assetData))
		return E_FAIL;

	if (FAILED(Ready_Meshes(assetData)))
		return E_FAIL;
	if (FAILED(Ready_Materials(assetData)))
		return E_FAIL;
	if (FAILED(Ready_Animation(assetData, desc.defaultAnimationName)))
		return E_FAIL;
	return S_OK;
}

HRESULT CCookedModel::Ready_Meshes(const MODEL_ASSET_DATA& assetData)
{
	if (assetData.meshes.empty())
		return E_FAIL;

	m_Meshes.clear();
	m_Meshes.reserve(assetData.meshes.size());
	m_IsSkinned = MODEL_VERTEX_KIND::SKINNED == assetData.meshes.front().vertexKind;

	for (const MODEL_MESH_DATA& source : assetData.meshes)
	{
		const bool_t skinned = MODEL_VERTEX_KIND::SKINNED == source.vertexKind;
		if (skinned != m_IsSkinned || source.indices.empty() || source.indices.size() > UINT32_MAX)
			return E_FAIL;

		const void* pVertices = nullptr;
		size_t vertexCount = {};
		uint32_t vertexStride = {};
		if (skinned)
		{
			pVertices = source.skinnedVertices.data();
			vertexCount = source.skinnedVertices.size();
			vertexStride = sizeof(VTXANIMMESH);
		}
		else
		{
			pVertices = source.vertices.data();
			vertexCount = source.vertices.size();
			vertexStride = sizeof(VTXMESH);
		}

		if (0 == vertexCount || vertexCount > UINT32_MAX)
			return E_FAIL;
		const uint64_t vertexBytes = vertexCount * vertexStride;
		const uint64_t indexBytes = source.indices.size() * sizeof(uint32_t);
		if (vertexBytes > UINT32_MAX || indexBytes > UINT32_MAX)
			return E_FAIL;

		RUNTIME_MESH mesh{};
		mesh.vertexStride = vertexStride;
		mesh.indexCount = static_cast<uint32_t>(source.indices.size());
		mesh.materialIndex = source.materialIndex;

		D3D11_BUFFER_DESC vertexDesc{};
		vertexDesc.ByteWidth = static_cast<uint32_t>(vertexBytes);
		vertexDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA vertexData{};
		vertexData.pSysMem = pVertices;
		if (FAILED(m_pDevice->CreateBuffer(&vertexDesc, &vertexData, &mesh.pVertexBuffer)))
			return E_FAIL;

		D3D11_BUFFER_DESC indexDesc{};
		indexDesc.ByteWidth = static_cast<uint32_t>(indexBytes);
		indexDesc.Usage = D3D11_USAGE_DEFAULT;
		indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA indexData{};
		indexData.pSysMem = source.indices.data();
		if (FAILED(m_pDevice->CreateBuffer(&indexDesc, &indexData, &mesh.pIndexBuffer)))
			return E_FAIL;

		m_Meshes.push_back(move(mesh));
	}

	return S_OK;
}

HRESULT CCookedModel::Ready_Materials(const MODEL_ASSET_DATA& assetData)
{
	vector<MODEL_MATERIAL_DATA> materials = assetData.materials;
	if (materials.empty())
		materials.emplace_back();

	m_Materials.clear();
	m_Materials.reserve(materials.size());
	for (const MODEL_MATERIAL_DATA& source : materials)
	{
		RUNTIME_MATERIAL material{};
		if (FAILED(LoadTexture(m_pDevice.Get(), source.diffusePath, 0xffffffff, material.pDiffuseSRV)))
			return E_FAIL;
		if (FAILED(LoadTexture(m_pDevice.Get(), source.normalPath, 0xffff8080, material.pNormalSRV)))
			return E_FAIL;
		m_Materials.push_back(move(material));
	}
	return S_OK;
}

HRESULT CCookedModel::Ready_Animation(const MODEL_ASSET_DATA& assetData,
	const string& defaultAnimationName)
{
	m_Skeleton = {};
	m_Animations.clear();
	m_LocalBoneMatrices.clear();
	m_GlobalBoneMatrices.clear();
	m_FinalBoneMatrices.clear();
	m_CurrentAnimationIndex = static_cast<uint32_t>(-1);
	m_CurrentTimeTicks = 0.f;
	m_IsPlaying = false;

	if (!m_IsSkinned)
		return assetData.hasSkeleton ? E_FAIL : S_OK;
	if (!assetData.hasSkeleton || assetData.skeleton.bones.empty() ||
		assetData.skeleton.bones.size() > 512)
		return E_FAIL;

	m_Skeleton = assetData.skeleton;
	m_Animations = assetData.animations;
	m_LocalBoneMatrices.resize(m_Skeleton.bones.size());
	m_GlobalBoneMatrices.resize(m_Skeleton.bones.size());
	m_FinalBoneMatrices.resize(m_Skeleton.bones.size());

	if (!m_Animations.empty())
	{
		uint32_t animationIndex = 0;
		if (!defaultAnimationName.empty())
		{
			const auto found = find_if(m_Animations.begin(), m_Animations.end(),
				[&defaultAnimationName](const MODEL_ANIMATION_DATA& animation)
				{
					return animation.name == defaultAnimationName;
				});
			if (found != m_Animations.end())
				animationIndex = static_cast<uint32_t>(distance(m_Animations.begin(), found));
		}
		Play_Animation(animationIndex, m_Animations[animationIndex].defaultLoop);
	}
	else
	{
		Evaluate_Pose();
	}
	return S_OK;
}

const string& CCookedModel::Get_AnimationName(uint32_t index) const
{
	static const string empty;
	return index < m_Animations.size() ? m_Animations[index].name : empty;
}

bool_t CCookedModel::Play_Animation(uint32_t index, bool_t loop)
{
	if (index >= m_Animations.size())
		return false;

	m_CurrentAnimationIndex = index;
	m_CurrentTimeTicks = 0.f;
	m_IsLoop = loop;
	m_IsPlaying = true;
	Evaluate_Pose();
	return true;
}

bool_t CCookedModel::Play_Animation(const string& name, bool_t loop)
{
	for (uint32_t i = 0; i < m_Animations.size(); ++i)
	{
		if (m_Animations[i].name == name)
			return Play_Animation(i, loop);
	}
	return false;
}

void CCookedModel::Stop_Animation()
{
	m_IsPlaying = false;
}

void CCookedModel::Update_Animation(f32_t fTimeDelta)
{
	if (!m_IsPlaying || m_CurrentAnimationIndex >= m_Animations.size() || !isfinite(fTimeDelta))
		return;

	const MODEL_ANIMATION_DATA& animation = m_Animations[m_CurrentAnimationIndex];
	m_CurrentTimeTicks += fTimeDelta * animation.ticksPerSecond * m_AnimationSpeed;
	if (m_IsLoop)
	{
		m_CurrentTimeTicks = fmod(m_CurrentTimeTicks, animation.durationTicks);
		if (m_CurrentTimeTicks < 0.f)
			m_CurrentTimeTicks += animation.durationTicks;
	}
	else if (m_CurrentTimeTicks >= animation.durationTicks)
	{
		m_CurrentTimeTicks = animation.durationTicks;
		m_IsPlaying = false;
	}
	else if (m_CurrentTimeTicks <= 0.f)
	{
		m_CurrentTimeTicks = 0.f;
		if (m_AnimationSpeed < 0.f)
			m_IsPlaying = false;
	}

	Evaluate_Pose();
}

void CCookedModel::Evaluate_Pose()
{
	if (m_Skeleton.bones.empty())
		return;

	for (uint32_t i = 0; i < m_Skeleton.bones.size(); ++i)
		m_LocalBoneMatrices[i] = m_Skeleton.bones[i].restLocal;

	if (m_CurrentAnimationIndex < m_Animations.size())
	{
		const MODEL_ANIMATION_DATA& animation = m_Animations[m_CurrentAnimationIndex];
		for (const MODEL_ANIMATION_CHANNEL_DATA& channel : animation.channels)
		{
			if (channel.resolvedBoneIndex < 0 ||
				channel.resolvedBoneIndex >= static_cast<int32_t>(m_Skeleton.bones.size()))
				continue;

			const uint32_t boneIndex = static_cast<uint32_t>(channel.resolvedBoneIndex);
			const matrix_t rest = XMLoadFloat4x4(&m_Skeleton.bones[boneIndex].restLocal);
			vector_t restScale = XMVectorSet(1.f, 1.f, 1.f, 0.f);
			vector_t restRotation = XMQuaternionIdentity();
			vector_t restTranslation = XMVectorZero();
			XMMatrixDecompose(&restScale, &restRotation, &restTranslation, rest);

			const vector_t position = SampleVector(
				channel.positionKeys, m_CurrentTimeTicks, restTranslation);
			const vector_t rotation = SampleQuaternion(
				channel.rotationKeys, m_CurrentTimeTicks, restRotation);
			const vector_t scale = SampleVector(
				channel.scaleKeys, m_CurrentTimeTicks, restScale);
			XMStoreFloat4x4(&m_LocalBoneMatrices[boneIndex],
				XMMatrixScalingFromVector(scale) *
				XMMatrixRotationQuaternion(rotation) *
				XMMatrixTranslationFromVector(position));
		}
	}

	const matrix_t globalInverseRoot = XMLoadFloat4x4(&m_Skeleton.globalInverseRoot);
	for (uint32_t i = 0; i < m_Skeleton.bones.size(); ++i)
	{
		const matrix_t local = XMLoadFloat4x4(&m_LocalBoneMatrices[i]);
		const int32_t parentIndex = m_Skeleton.bones[i].parentIndex;
		const matrix_t global = parentIndex >= 0
			? local * XMLoadFloat4x4(&m_GlobalBoneMatrices[parentIndex])
			: local;
		XMStoreFloat4x4(&m_GlobalBoneMatrices[i], global);

		const matrix_t inverseBind = XMLoadFloat4x4(&m_Skeleton.bones[i].inverseBind);
		XMStoreFloat4x4(&m_FinalBoneMatrices[i], inverseBind * global * globalInverseRoot);
	}
}

HRESULT CCookedModel::Bind_Material(shared_ptr<CShader> pShader,
	const char_t* pConstantName,
	uint32_t meshIndex,
	COOKED_TEXTURE_SLOT slot) const
{
	if (nullptr == pShader || nullptr == pConstantName || meshIndex >= m_Meshes.size())
		return E_FAIL;

	const uint32_t materialIndex = m_Meshes[meshIndex].materialIndex;
	if (materialIndex >= m_Materials.size())
		return E_FAIL;

	const RUNTIME_MATERIAL& material = m_Materials[materialIndex];
	switch (slot)
	{
	case COOKED_TEXTURE_SLOT::DIFFUSE:
		return pShader->Bind_Texture(pConstantName, material.pDiffuseSRV);
	case COOKED_TEXTURE_SLOT::NORMAL:
		return pShader->Bind_Texture(pConstantName, material.pNormalSRV);
	default:
		return E_FAIL;
	}
}

HRESULT CCookedModel::Bind_BoneMatrices(shared_ptr<CShader> pShader,
	const char_t* pConstantName) const
{
	if (nullptr == pShader || nullptr == pConstantName || m_FinalBoneMatrices.empty())
		return E_FAIL;
	return pShader->Bind_Matrices(
		pConstantName,
		m_FinalBoneMatrices.data(),
		static_cast<uint32_t>(m_FinalBoneMatrices.size()));
}

HRESULT CCookedModel::Render(uint32_t meshIndex) const
{
	if (meshIndex >= m_Meshes.size())
		return E_FAIL;

	const RUNTIME_MESH& mesh = m_Meshes[meshIndex];
	const uint32_t offset = {};
	ID3D11Buffer* pVertexBuffer = mesh.pVertexBuffer.Get();
	m_pContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &mesh.vertexStride, &offset);
	m_pContext->IASetIndexBuffer(mesh.pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_pContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_pContext->DrawIndexed(mesh.indexCount, 0, 0);
	return S_OK;
}

shared_ptr<CCookedModel> CCookedModel::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext,
	const MODEL_ASSET_LOAD_DESC& desc)
{
	auto pInstance = shared_ptr<CCookedModel>(new CCookedModel(pDevice, pContext));
	if (FAILED(pInstance->Initialize(desc)))
		return nullptr;
	return pInstance;
}
