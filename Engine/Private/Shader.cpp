#include "Shader.h"
#include "GameInstance.h"
#include "Profiler.h"

#include <cwchar>
#include <cstring>
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
		uint64_t iElapsedMs,
		uint32_t iPassCount = 0u,
		uint32_t iCreatedInputLayouts = 0u)
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
		if (0u != iPassCount)
		{
			Message += L" passes=" + std::to_wstring(iPassCount);
			Message += L" inputLayoutsCreated=" + std::to_wstring(iCreatedInputLayouts);
		}
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
	CProfiler* const pProfiler = CGameInstance::Get().Get_Profiler();
	CProfilerScope loadScope(pProfiler, "Shader.Load");
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
	{
		CProfilerScope scope(pProfiler, "Shader.ReadBytecode");
		Result = ReadCompiledEffect(CompiledPath, Bytecode, ByteCount);
	}
	if (FAILED(Result))
		return Fail(L"read", Result);

	ComPtr<ID3DX11Effect> Effect;
	{
		CProfilerScope scope(pProfiler, "Shader.CreateEffect");
		Result = D3DX11CreateEffectFromMemory(
			Bytecode.data(),
			Bytecode.size(),
			0u,
			m_pDevice.Get(),
			Effect.GetAddressOf());
	}
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

	// Input elements are fixed for this prototype. Identical byte signatures
	// therefore describe the same immutable D3D input layout across its passes.
	struct INPUT_SIGNATURE_LAYOUT final
	{
		const void* pBytes = nullptr;
		size_t iByteCount = 0u;
		size_t iLayoutIndex = 0u;
	};
	std::vector<INPUT_SIGNATURE_LAYOUT> UniqueInputSignatures;
	std::vector<ComPtr<ID3D11InputLayout>> InputLayouts;
	std::shared_ptr<EFFECT_BINDINGS> Bindings;
	try
	{
		CProfilerScope scope(pProfiler, "Shader.BuildBindings");
		UniqueInputSignatures.reserve(TechniqueDesc.Passes);
		InputLayouts.reserve(TechniqueDesc.Passes);
		Bindings = std::make_shared<EFFECT_BINDINGS>();
		Bindings->Passes.reserve(TechniqueDesc.Passes);
		size_t VariableCapacity = 2u;
		while (VariableCapacity < static_cast<size_t>(EffectDesc.GlobalVariables) * 2u)
			VariableCapacity *= 2u;
		Bindings->Variables.resize(VariableCapacity);
		Bindings->iVariableMask = VariableCapacity - 1u;
		for (uint32_t i = 0u; i < EffectDesc.GlobalVariables; ++i)
		{
			ID3DX11EffectVariable* pIndexedVariable = Effect->GetVariableByIndex(i);
			if (nullptr == pIndexedVariable || !pIndexedVariable->IsValid())
				return Fail(L"variable", E_FAIL);
			D3DX11_EFFECT_VARIABLE_DESC VariableDesc{};
			Result = pIndexedVariable->GetDesc(&VariableDesc);
			if (FAILED(Result))
				return Fail(L"variable-desc", Result);
			if (nullptr == VariableDesc.Name || '\0' == VariableDesc.Name[0])
				continue;

			// Resolve the public name once, including the Effects name lookup's
			// own precedence. Drawing uses only this immutable per-Effect table.
			ID3DX11EffectVariable* pVariable = Effect->GetVariableByName(VariableDesc.Name);
			if (nullptr == pVariable || !pVariable->IsValid())
				return Fail(L"variable-name", E_FAIL);
			const uint64_t NameHash = Hash_VariableName(VariableDesc.Name);
			size_t Slot = static_cast<size_t>(NameHash) & Bindings->iVariableMask;
			while (nullptr != Bindings->Variables[Slot].pVariable &&
				std::strcmp(Bindings->Variables[Slot].strName.c_str(), VariableDesc.Name) != 0)
				Slot = (Slot + 1u) & Bindings->iVariableMask;
			VARIABLE_BINDING& Binding = Bindings->Variables[Slot];
			if (nullptr != Binding.pVariable)
				continue;
			Binding.strName = VariableDesc.Name;
			Binding.iNameHash = NameHash;
			Binding.pVariable = pVariable;
			Binding.pMatrix = pVariable->AsMatrix();
			if (nullptr != Binding.pMatrix && !Binding.pMatrix->IsValid())
				Binding.pMatrix = nullptr;
			Binding.pResource = pVariable->AsShaderResource();
			if (nullptr != Binding.pResource && !Binding.pResource->IsValid())
				Binding.pResource = nullptr;
		}
	}
	catch (const std::bad_alloc&)
	{
		return Fail(L"binding-cache-reserve", E_OUTOFMEMORY);
	}

	{
		CProfilerScope scope(pProfiler, "Shader.CreateInputLayouts");
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

			const auto cached = std::find_if(UniqueInputSignatures.begin(), UniqueInputSignatures.end(),
				[&PassDesc](const INPUT_SIGNATURE_LAYOUT& signature)
				{
					return signature.iByteCount == PassDesc.IAInputSignatureSize &&
						0 == std::memcmp(signature.pBytes, PassDesc.pIAInputSignature, signature.iByteCount);
				});
			if (cached != UniqueInputSignatures.end())
			{
				InputLayouts.push_back(InputLayouts[cached->iLayoutIndex]);
				Bindings->Passes.push_back(pPass);
				continue;
			}

			ComPtr<ID3D11InputLayout> InputLayout;
			Result = m_pDevice->CreateInputLayout(
				pElements,
				iNumElements,
				PassDesc.pIAInputSignature,
				PassDesc.IAInputSignatureSize,
				InputLayout.GetAddressOf());
			if (FAILED(Result) || nullptr == InputLayout)
				return Fail(L"input-layout", FAILED(Result) ? Result : E_FAIL);

			UniqueInputSignatures.push_back({ PassDesc.pIAInputSignature,
				PassDesc.IAInputSignatureSize, InputLayouts.size() });
			InputLayouts.push_back(std::move(InputLayout));
			Bindings->Passes.push_back(pPass);
		}
	}

	m_pEffect = std::move(Effect);
	m_InputLayouts = std::move(InputLayouts);
	m_pBindings = std::move(Bindings);
	m_iNumPasses = TechniqueDesc.Passes;

	TraceCompiledEffectLoad(
		L"complete",
		pShaderFilePath,
		ModulePath,
		CompiledPath,
		ByteCount,
		S_OK,
		GetTickCount64() - StartTime,
		TechniqueDesc.Passes,
		static_cast<uint32_t>(UniqueInputSignatures.size()));
	return S_OK;
}

HRESULT CShader::Initialize(void* pArg)
{
	return S_OK;
}

const CShader::VARIABLE_BINDING* CShader::Find_Variable(const char_t* pConstantName) const
{
	const EFFECT_BINDINGS* pBindings = m_pBindings.get();
	if (nullptr == pBindings || nullptr == pConstantName || '\0' == pConstantName[0])
		return nullptr;
	const uint64_t NameHash = Hash_VariableName(pConstantName);
	size_t Slot = static_cast<size_t>(NameHash) & pBindings->iVariableMask;
	const VARIABLE_BINDING* pVariables = pBindings->Variables.data();
	while (nullptr != pVariables[Slot].pVariable)
	{
		if (pVariables[Slot].iNameHash == NameHash &&
			std::strcmp(pVariables[Slot].strName.c_str(), pConstantName) == 0)
			return &pVariables[Slot];
		Slot = (Slot + 1u) & pBindings->iVariableMask;
	}
	return nullptr;
}

uint64_t CShader::Hash_VariableName(const char_t* pConstantName) noexcept
{
	uint64_t Hash = 14695981039346656037ull;
	for (; '\0' != *pConstantName; ++pConstantName)
	{
		Hash ^= static_cast<unsigned char>(*pConstantName);
		Hash *= 1099511628211ull;
	}
	return Hash;
}

HRESULT CShader::Bind_RawValue(const char_t* pConstantName, const void* pData, uint32_t iLength)
{
	const VARIABLE_BINDING* pBinding = Find_Variable(pConstantName);
	if (nullptr == pBinding)
		return E_FAIL;

	return pBinding->pVariable->SetRawValue(pData, 0, iLength);
}

HRESULT CShader::Bind_Matrix(const char_t* pConstantName, const float4x4_t* pMatrix)
{
	const VARIABLE_BINDING* pBinding = Find_Variable(pConstantName);
	if (nullptr == pBinding || nullptr == pBinding->pMatrix)
		return E_FAIL;

	return pBinding->pMatrix->SetMatrix(reinterpret_cast<const float_t*>(pMatrix));
}

HRESULT CShader::Bind_Matrices(const char_t* pConstantName, const float4x4_t* pMatrices, uint32_t iNumMatrices)
{
	const VARIABLE_BINDING* pBinding = Find_Variable(pConstantName);
	if (nullptr == pBinding || nullptr == pBinding->pMatrix)
		return E_FAIL;

	return pBinding->pMatrix->SetMatrixArray(reinterpret_cast<const float_t*>(pMatrices), 0, iNumMatrices);
}

HRESULT CShader::Bind_Texture(const char_t* pConstantName, ComPtr<ID3D11ShaderResourceView> pSRV)
{
	const VARIABLE_BINDING* pBinding = Find_Variable(pConstantName);
	if (nullptr == pBinding || nullptr == pBinding->pResource)
		return E_FAIL;

	return pBinding->pResource->SetResource(pSRV.Get());
}

HRESULT CShader::Bind_Textures(const char_t* pConstantName, ID3D11ShaderResourceView** ppSRV, uint32_t iNumSRVs)
{
	const VARIABLE_BINDING* pBinding = Find_Variable(pConstantName);
	if (nullptr == pBinding || nullptr == pBinding->pResource)
		return E_FAIL;

	return pBinding->pResource->SetResourceArray(ppSRV, 0, iNumSRVs);
}



HRESULT CShader::Begin(uint32_t iPassIndex)
{
	if (nullptr == m_pBindings || iPassIndex >= m_iNumPasses)
		return E_FAIL;

	m_pContext->IASetInputLayout(m_InputLayouts[iPassIndex].Get());

	return m_pBindings->Passes[iPassIndex]->Apply(0, m_pContext.Get());
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
