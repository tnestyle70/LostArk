#pragma once

#include "Component.h"

#include <memory>
#include <string>
#include <vector>

struct ID3DX11Effect;
struct ID3DX11EffectVariable;
struct ID3DX11EffectMatrixVariable;
struct ID3DX11EffectShaderResourceVariable;
struct ID3DX11EffectPass;

NS_BEGIN(Engine)

class ENGINE_DLL CShader final : public CComponent
{
private:
	CShader(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	CShader(const CShader& shader);
	CShader& operator=(const CShader& shader);
	virtual ~CShader();
public:
	virtual HRESULT Initialize_Prototype(const tchar_t* pShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* pElement, uint32_t iNumElements);
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Bind_RawValue(const char_t* pConstantName, const void* pData, uint32_t iLength);
	HRESULT Bind_Matrix(const char_t* pConstantName, const float4x4_t* pMatrix);
	HRESULT Bind_Matrices(const char_t* pConstantName, const float4x4_t* pMatrices, uint32_t iNumMatrices);
	HRESULT Bind_Texture(const char_t* pConstantName, const ComPtr<ID3D11ShaderResourceView>& pSRV);
	HRESULT Bind_Textures(const char_t* pConstantName, ID3D11ShaderResourceView** ppSRV, uint32_t iNumSRVs);
	HRESULT Begin(uint32_t iPassIndex);

private:
	struct VARIABLE_BINDING final
	{
		std::string strName;
		uint64_t iNameHash = 0u;
		ID3DX11EffectVariable* pVariable = nullptr;
		ID3DX11EffectMatrixVariable* pMatrix = nullptr;
		ID3DX11EffectShaderResourceVariable* pResource = nullptr;
		enum class VALUE_KIND : uint8_t { NONE, RAW, MATRIX };
		// This state belongs to the Effect, so every Clone observes every setter.
		uint8_t LastValue[sizeof(float4x4_t)]{};
		uint32_t iLastValueBytes = 0u;
		VALUE_KIND eLastValueKind = VALUE_KIND::NONE;
		ID3D11ShaderResourceView* pLastResource = nullptr;
		bool bHasLastResource = false;
	};
	struct EFFECT_BINDINGS final
	{
		// A half-full table avoids Debug STL lookup/iterator work
		// in every draw. Empty slots terminate probing; keys own their bytes.
		std::vector<VARIABLE_BINDING> Variables;
		size_t iVariableMask = 0u;
		std::vector<ID3DX11EffectPass*> Passes;
	};

	VARIABLE_BINDING* Find_Variable(const char_t* pConstantName) const;
	static uint64_t Hash_VariableName(const char_t* pConstantName) noexcept;

	ComPtr<ID3DX11Effect>				m_pEffect = { nullptr };
	vector<ComPtr<ID3D11InputLayout>>	m_InputLayouts;
	// Handles and last successful inputs share the same owner as m_pEffect.
	// Every variable write stays private to CShader; Begin always applies the pass.
	std::shared_ptr<EFFECT_BINDINGS> m_pBindings;

private:
	uint32_t			m_iNumPasses = {};

public:
	static unique_ptr<CShader> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const tchar_t* pShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* pElement, uint32_t iNumElements);
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;

};

NS_END
