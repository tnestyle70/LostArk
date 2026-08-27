#include "Shader.h"

#include <cwchar>
#include <new>

namespace
{
	constexpr uint64_t MaxCompiledEffectBytes = 256ull * 1024ull * 1024ull;
	constexpr uint32_t MaxCompiledEffectPasses = 1024u;

	void TraceCompiledEffectLoad(
		const wchar_t* pStage,
		const wchar_t* pLogicalPath,
		const std::wstring& ModulePath,
		const std::wstring& CompiledPath,
		uint64_t iByteCount,
		HRESULT hResult,
		uint64_t iElapsedMs)
	{
		wchar_t ResultText[16]{};
		swprintf_s(ResultText, L"0x%08X", static_cast<uint32_t>(hResult));

		std::wstring Message = L"[Engine][Shader] compiled-effect ";
		Message += SUCCEEDED(hResult) ? L"loaded" : L"failed";
		Message += L" stage=";
		Message += nullptr != pStage ? pStage : L"unknown";
		Message += L" logical=";
		Message += nullptr != pLogicalPath ? pLogicalPath : L"<null>";
		Message += L" module=";
		Message += ModulePath.empty() ? L"<unresolved>" : ModulePath;
		Message += L" cso=";
		Message += CompiledPath.empty() ? L"<unresolved>" : CompiledPath;
		Message += L" bytes=";
		Message += std::to_wstring(iByteCount);
		Message += L" hr=";
		Message += ResultText;
		Message += L" elapsedMs=";
		Message += std::to_wstring(iElapsedMs);
		Message += L"\n";
		OutputDebugStringW(Message.c_str());
	}

	HRESULT ResolveCompiledEffectPath(
		const wchar_t* pLogicalPath,
		std::wstring& ModulePath,
		std::wstring& CompiledPath)
	{
		if (nullptr == pLogicalPath || L'\0' == pLogicalPath[0])
			return E_INVALIDARG;

		wchar_t ModuleBuffer[32768]{};
		const DWORD ModuleLength = GetModuleFileNameW(
			nullptr,
			ModuleBuffer,
			static_cast<DWORD>(_countof(ModuleBuffer)));
		if (0 == ModuleLength || ModuleLength >= _countof(ModuleBuffer))
			return HRESULT_FROM_WIN32(0 != GetLastError() ? GetLastError() : ERROR_INSUFFICIENT_BUFFER);

		ModulePath.assign(ModuleBuffer, ModuleLength);
		const size_t ModuleSeparator = ModulePath.find_last_of(L"\\/");
		if (std::wstring::npos == ModuleSeparator)
			return HRESULT_FROM_WIN32(ERROR_INVALID_NAME);

		const std::wstring LogicalPath = pLogicalPath;
		const size_t LogicalSeparator = LogicalPath.find_last_of(L"\\/");
		const size_t FileNameStart = std::wstring::npos == LogicalSeparator ? 0u : LogicalSeparator + 1u;
		const size_t Extension = LogicalPath.find_last_of(L'.');
		if (FileNameStart >= LogicalPath.size() ||
			std::wstring::npos == Extension ||
			Extension <= FileNameStart)
		{
			return HRESULT_FROM_WIN32(ERROR_INVALID_NAME);
		}

		CompiledPath.assign(ModulePath, 0u, ModuleSeparator + 1u);
		CompiledPath.append(LogicalPath, FileNameStart, Extension - FileNameStart);
		CompiledPath += L".cso";
		return S_OK;
	}

	HRESULT ReadCompiledEffect(
		const std::wstring& CompiledPath,
		std::vector<uint8_t>& Bytecode,
		uint64_t& iByteCount)
	{
		iByteCount = 0u;
		Bytecode.clear();

		const HANDLE FileHandle = CreateFileW(
			CompiledPath.c_str(),
			GENERIC_READ,
			FILE_SHARE_READ,
			nullptr,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
			nullptr);
		if (INVALID_HANDLE_VALUE == FileHandle)
			return HRESULT_FROM_WIN32(GetLastError());

		LARGE_INTEGER FileSize{};
		if (FALSE == GetFileSizeEx(FileHandle, &FileSize))
		{
			const HRESULT Result = HRESULT_FROM_WIN32(GetLastError());
			CloseHandle(FileHandle);
			return Result;
		}

		if (FileSize.QuadPart <= 0)
		{
			CloseHandle(FileHandle);
			return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		}
		if (static_cast<uint64_t>(FileSize.QuadPart) > MaxCompiledEffectBytes)
		{
			CloseHandle(FileHandle);
			return HRESULT_FROM_WIN32(ERROR_FILE_TOO_LARGE);
		}

		iByteCount = static_cast<uint64_t>(FileSize.QuadPart);
		try
		{
			Bytecode.resize(static_cast<size_t>(iByteCount));
		}
		catch (const std::bad_alloc&)
		{
			CloseHandle(FileHandle);
			return E_OUTOFMEMORY;
		}

		DWORD BytesRead = 0u;
		const BOOL ReadSucceeded = ReadFile(
			FileHandle,
			Bytecode.data(),
			static_cast<DWORD>(Bytecode.size()),
			&BytesRead,
			nullptr);
		const DWORD ReadError = FALSE == ReadSucceeded ? GetLastError() : ERROR_SUCCESS;
		CloseHandle(FileHandle);

		if (FALSE == ReadSucceeded)
		{
			Bytecode.clear();
			return HRESULT_FROM_WIN32(ReadError);
		}
		if (BytesRead != Bytecode.size())
		{
			Bytecode.clear();
			return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
		}

		return S_OK;
	}
}

CShader::CShader(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CComponent { pDevice, pContext }
{
}

CShader::~CShader()
{
}

HRESULT CShader::Initialize_Prototype(const tchar_t* pShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* pElements, uint32_t iNumElements)
{
	const uint64_t StartTime = GetTickCount64();
	std::wstring ModulePath;
	std::wstring CompiledPath;
	uint64_t ByteCount = 0u;

	auto Fail = [&](const wchar_t* pStage, HRESULT Result) -> HRESULT
	{
		TraceCompiledEffectLoad(
			pStage,
			pShaderFilePath,
			ModulePath,
			CompiledPath,
			ByteCount,
			Result,
			GetTickCount64() - StartTime);
		return FAILED(Result) ? Result : E_FAIL;
	};

	if (nullptr == m_pDevice || nullptr == pElements || 0u == iNumElements)
		return Fail(L"arguments", E_INVALIDARG);

	HRESULT Result = ResolveCompiledEffectPath(pShaderFilePath, ModulePath, CompiledPath);
	if (FAILED(Result))
		return Fail(L"resolve", Result);

	std::vector<uint8_t> Bytecode;
	Result = ReadCompiledEffect(CompiledPath, Bytecode, ByteCount);
	if (FAILED(Result))
		return Fail(L"read", Result);

	ComPtr<ID3DX11Effect> Effect;
	Result = D3DX11CreateEffectFromMemory(
		Bytecode.data(),
		Bytecode.size(),
		0u,
		m_pDevice.Get(),
		Effect.GetAddressOf());
	if (FAILED(Result) || nullptr == Effect || !Effect->IsValid())
		return Fail(L"effect", FAILED(Result) ? Result : E_FAIL);

	D3DX11_EFFECT_DESC EffectDesc{};
	Result = Effect->GetDesc(&EffectDesc);
	if (FAILED(Result) || 0u == EffectDesc.Techniques)
		return Fail(L"effect-desc", FAILED(Result) ? Result : E_FAIL);

	ID3DX11EffectTechnique* pTechnique = Effect->GetTechniqueByIndex(0u);
	if (nullptr == pTechnique || !pTechnique->IsValid())
		return Fail(L"technique", E_FAIL);

	D3DX11_TECHNIQUE_DESC TechniqueDesc{};
	Result = pTechnique->GetDesc(&TechniqueDesc);
	if (FAILED(Result) || 0u == TechniqueDesc.Passes || TechniqueDesc.Passes > MaxCompiledEffectPasses)
		return Fail(L"technique-desc", FAILED(Result) ? Result : E_FAIL);

	std::vector<ComPtr<ID3D11InputLayout>> InputLayouts;
	try
	{
		InputLayouts.reserve(TechniqueDesc.Passes);
	}
	catch (const std::bad_alloc&)
	{
		return Fail(L"input-layout-reserve", E_OUTOFMEMORY);
	}

	for (uint32_t i = 0u; i < TechniqueDesc.Passes; ++i)
	{
		ID3DX11EffectPass* pPass = pTechnique->GetPassByIndex(i);
		if (nullptr == pPass || !pPass->IsValid())
			return Fail(L"pass", E_FAIL);

		D3DX11_PASS_DESC PassDesc{};
		Result = pPass->GetDesc(&PassDesc);
		if (FAILED(Result))
			return Fail(L"pass-desc", Result);
		if (nullptr == PassDesc.pIAInputSignature || 0u == PassDesc.IAInputSignatureSize)
			return Fail(L"pass-signature", E_FAIL);

		ComPtr<ID3D11InputLayout> InputLayout;
		Result = m_pDevice->CreateInputLayout(
			pElements,
			iNumElements,
			PassDesc.pIAInputSignature,
			PassDesc.IAInputSignatureSize,
			InputLayout.GetAddressOf());
		if (FAILED(Result) || nullptr == InputLayout)
			return Fail(L"input-layout", FAILED(Result) ? Result : E_FAIL);

		InputLayouts.push_back(std::move(InputLayout));
	}

	m_pEffect = std::move(Effect);
	m_InputLayouts = std::move(InputLayouts);
	m_iNumPasses = TechniqueDesc.Passes;

	TraceCompiledEffectLoad(
		L"complete",
		pShaderFilePath,
		ModulePath,
		CompiledPath,
		ByteCount,
		S_OK,
		GetTickCount64() - StartTime);
	return S_OK;
}

HRESULT CShader::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CShader::Bind_RawValue(const char_t* pConstantName, const void* pData, uint32_t iLength)
{
	ComPtr<ID3DX11EffectVariable>	pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	return pVariable->SetRawValue(pData, 0, iLength);	
}

HRESULT CShader::Bind_Matrix(const char_t* pConstantName, const float4x4_t* pMatrix)
{
	ComPtr<ID3DX11EffectVariable>	pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	ComPtr<ID3DX11EffectMatrixVariable>		pMatrixVariable = pVariable->AsMatrix();
	if (nullptr == pMatrixVariable)
		return E_FAIL;

	return pMatrixVariable->SetMatrix(reinterpret_cast<const float_t*>(pMatrix));	
}

HRESULT CShader::Bind_Matrices(const char_t* pConstantName, const float4x4_t* pMatrices, uint32_t iNumMatrices)
{
	ComPtr<ID3DX11EffectVariable>	pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	ComPtr<ID3DX11EffectMatrixVariable>		pMatrixVariable = pVariable->AsMatrix();
	if (nullptr == pMatrixVariable)
		return E_FAIL;

	return pMatrixVariable->SetMatrixArray(reinterpret_cast<const float_t*>(pMatrices), 0, iNumMatrices);
}

HRESULT CShader::Bind_Texture(const char_t* pConstantName, ComPtr<ID3D11ShaderResourceView> pSRV)
{
	ComPtr<ID3DX11EffectVariable>	pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	ComPtr<ID3DX11EffectShaderResourceVariable>	pSRVVariable = pVariable->AsShaderResource();
	if (nullptr == pSRVVariable)
		return E_FAIL;

	return pSRVVariable->SetResource(pSRV.Get());	
}

HRESULT CShader::Bind_Textures(const char_t* pConstantName, ID3D11ShaderResourceView** ppSRV, uint32_t iNumSRVs)
{
	ComPtr<ID3DX11EffectVariable>	pVariable = m_pEffect->GetVariableByName(pConstantName);
	if (nullptr == pVariable)
		return E_FAIL;

	ComPtr<ID3DX11EffectShaderResourceVariable>	pSRVVariable = pVariable->AsShaderResource();
	if (nullptr == pSRVVariable)
		return E_FAIL;

	return pSRVVariable->SetResourceArray(ppSRV, 0, iNumSRVs);
}



HRESULT CShader::Begin(uint32_t iPassIndex)
{
	if (iPassIndex >= m_iNumPasses)
		return E_FAIL;

	m_pContext->IASetInputLayout(m_InputLayouts[iPassIndex].Get());

	return m_pEffect->GetTechniqueByIndex(0)->GetPassByIndex(iPassIndex)->Apply(0, m_pContext.Get());	
}

unique_ptr<CShader> CShader::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const tchar_t* pShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* pElements, uint32_t iNumElements)
{
	auto pInstance = unique_ptr<CShader>(new CShader(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype(pShaderFilePath, pElements, iNumElements)))
	{
		OutputDebugStringA("[Engine][Shader] Create failed.\n");
		return nullptr;
	}

	return pInstance;
}


shared_ptr<CPrototype> CShader::Clone(void* pArg)
{
	auto pInstance = shared_ptr<CShader>(new CShader(*this));

	if (FAILED(pInstance->Initialize(pArg)))
	{
		OutputDebugStringA("[Engine][Shader] Clone failed.\n");
		return nullptr;
	}

	return pInstance;
}
