#include "Shader.h"
#pragma push_macro("new")
#undef new
#include "Engine_RenderTypes.h"
#pragma pop_macro("new")
#pragma push_macro("new")
#undef new
#include "Fx11/d3dx11effect.h"
#pragma pop_macro("new")
#include "GameInstance.h"
#include "Profiler.h"
#include "Render_OutputContract.h"

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

struct CShader::PROGRAM_VARIANTS final
{
	struct VARIABLE_COPY final
	{
		VARIABLE_BINDING* pSource = nullptr;
		VARIABLE_BINDING* pDestination = nullptr;
		uint64_t iCopiedRevision = 0u;
		std::vector<uint8_t> Bytes;
		std::vector<ID3D11ShaderResourceView*> Resources;
	};
	struct VARIANT final
	{
		uint32_t iFirstProgram = 0u;
		uint32_t iLastProgram = 0u;
		std::shared_ptr<CShader> pShader;
		std::vector<VARIABLE_COPY> Variables;
		uint64_t iCopiedRevision = 0u;
	};
	VARIABLE_BINDING* pProgram = nullptr;
	std::vector<VARIANT> Groups;
};

CShader::CShader(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CComponent { pDevice, pContext }
{
}

CShader::CShader(const CShader& shader) = default;

CShader& CShader::operator=(const CShader& shader) = default;

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

	std::shared_ptr<PROGRAM_VARIANTS> ProgramVariants;
	Result = Stage_ProgramVariants(pShaderFilePath, pElements, iNumElements, Bindings, ProgramVariants);
	if (FAILED(Result))
		return Fail(L"program-variants", Result);

	m_pEffect = std::move(Effect);
	m_InputLayouts = std::move(InputLayouts);
	m_pBindings = std::move(Bindings);
	m_pProgramVariants = std::move(ProgramVariants);
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

HRESULT CShader::Stage_ProgramVariants(const tchar_t* pShaderFilePath,
	const D3D11_INPUT_ELEMENT_DESC* pElements, uint32_t iNumElements,
	const std::shared_ptr<EFFECT_BINDINGS>& pBindings,
	std::shared_ptr<PROGRAM_VARIANTS>& pVariants)
{
	const std::wstring path = pShaderFilePath;
	const size_t separator = path.find_last_of(L"\\/");
	const std::wstring filename = path.substr(separator == std::wstring::npos ? 0u : separator + 1u);
	if (0 != _wcsicmp(filename.c_str(), L"Shader_VtxAnimMeshBinary.hlsl") &&
		0 != _wcsicmp(filename.c_str(), L"Shader_VtxMeshBinary.hlsl") &&
		0 != _wcsicmp(filename.c_str(), L"Shader_Deferred.hlsl"))
		return S_OK;
	try
	{
		auto staged = std::make_shared<PROGRAM_VARIANTS>();
		for (auto& variable : pBindings->Variables)
			if (variable.strName == "g_SourceCharacterProgram") staged->pProgram = &variable;
		if (nullptr == staged->pProgram) return E_FAIL;
		constexpr uint32_t ranges[][2] = { {1u,8u}, {9u,16u}, {17u,24u}, {25u,32u}, {80u,83u}, {84u,88u} };
		staged->Groups.reserve(std::size(ranges));
		for (const auto& range : ranges)
		{
			wchar_t suffix[40]{};
			swprintf_s(suffix, L"_SourceGroup%03u.hlsl", range[0]);
			const std::wstring variantPath = path.substr(0u, path.size() - 5u) + suffix;
			PROGRAM_VARIANTS::VARIANT variant;
			variant.iFirstProgram = range[0];
			variant.iLastProgram = range[1];
			variant.pShader = CShader::Create(m_pDevice, m_pContext, variantPath.c_str(), pElements, iNumElements);
			if (!variant.pShader || variant.pShader->m_iNumPasses != pBindings->Passes.size()) return E_FAIL;
			for (size_t pass = 0u; pass < pBindings->Passes.size(); ++pass)
			{
				D3DX11_PASS_DESC source{}, destination{};
				if (FAILED(pBindings->Passes[pass]->GetDesc(&source)) ||
					FAILED(variant.pShader->m_pBindings->Passes[pass]->GetDesc(&destination)) ||
					nullptr == source.Name || nullptr == destination.Name ||
					0 != std::strcmp(source.Name, destination.Name) ||
					nullptr == source.pIAInputSignature || nullptr == destination.pIAInputSignature ||
					0u == source.IAInputSignatureSize || source.IAInputSignatureSize != destination.IAInputSignatureSize ||
					0 != std::memcmp(source.pIAInputSignature, destination.pIAInputSignature, source.IAInputSignatureSize))
					return E_FAIL;
			}
			variant.Variables.reserve(pBindings->Variables.size() / 2u);
			for (auto& source : pBindings->Variables)
			{
				if (nullptr == source.pVariable) continue;
				D3DX11_EFFECT_TYPE_DESC sourceType{};
				if (FAILED(source.pVariable->GetType()->GetDesc(&sourceType))) return E_FAIL;
				// Shader and state objects belong to each compiled variant. Only caller
				// controlled constants and shader resources cross the FX boundary.
				if (nullptr == source.pResource && sourceType.Class == D3D_SVC_OBJECT) continue;
				auto* destination = variant.pShader->Find_Variable(source.strName.c_str());
				D3DX11_EFFECT_TYPE_DESC destinationType{};
				if (nullptr == destination || FAILED(destination->pVariable->GetType()->GetDesc(&destinationType)) ||
					sourceType.Class != destinationType.Class || sourceType.Type != destinationType.Type ||
					sourceType.Elements != destinationType.Elements || sourceType.Rows != destinationType.Rows ||
					sourceType.Columns != destinationType.Columns || sourceType.Members != destinationType.Members ||
					sourceType.UnpackedSize != destinationType.UnpackedSize ||
					(nullptr != source.pResource) != (nullptr != destination->pResource)) return E_FAIL;
				PROGRAM_VARIANTS::VARIABLE_COPY copy;
				copy.pSource = &source;
				copy.pDestination = destination;
				if (source.pResource) copy.Resources.resize(std::max(1u, sourceType.Elements));
				else copy.Bytes.resize(sourceType.UnpackedSize);
				variant.Variables.push_back(std::move(copy));
			}
			staged->Groups.push_back(std::move(variant));
		}
		pVariants = std::move(staged);
	}
	catch (const std::bad_alloc&)
	{
		return E_OUTOFMEMORY;
	}
	return S_OK;
}

HRESULT CShader::Apply_ProgramVariant(uint32_t iProgram, uint32_t iPassIndex)
{
	for (auto& variant : m_pProgramVariants->Groups)
	{
		if (iProgram < variant.iFirstProgram || iProgram > variant.iLastProgram) continue;
		if (variant.iCopiedRevision == m_pBindings->iRevision)
			return variant.pShader->Begin(iPassIndex);
		auto* copies = variant.Variables.data();
		for (size_t variableIndex = 0u; variableIndex < variant.Variables.size(); ++variableIndex)
		{
			auto& copy = copies[variableIndex];
			if (copy.iCopiedRevision == copy.pSource->iRevision) continue;
			HRESULT result = S_OK;
			if (!copy.Resources.empty())
			{
				const uint32_t count = static_cast<uint32_t>(copy.Resources.size());
				result = copy.pSource->pResource->GetResourceArray(copy.Resources.data(), 0u, count);
				if (SUCCEEDED(result)) result = copy.pDestination->pResource->SetResourceArray(copy.Resources.data(), 0u, count);
				for (auto*& resource : copy.Resources)
				{
					if (resource) resource->Release();
					resource = nullptr;
				}
				copy.pDestination->bHasLastResource = false;
			}
			else if (!copy.Bytes.empty())
			{
				const uint32_t count = static_cast<uint32_t>(copy.Bytes.size());
				result = copy.pSource->pVariable->GetRawValue(copy.Bytes.data(), 0u, count);
				if (SUCCEEDED(result)) result = copy.pDestination->pVariable->SetRawValue(copy.Bytes.data(), 0u, count);
				copy.pDestination->eLastValueKind = VARIABLE_BINDING::VALUE_KIND::NONE;
			}
			if (FAILED(result)) return result;
			copy.iCopiedRevision = copy.pSource->iRevision;
		}
		variant.iCopiedRevision = m_pBindings->iRevision;
		return variant.pShader->Begin(iPassIndex);
	}
	return E_INVALIDARG;
}

CShader::VARIABLE_BINDING* CShader::Find_Variable(const char_t* pConstantName) const
{
	EFFECT_BINDINGS* pBindings = m_pBindings.get();
	if (nullptr == pBindings || nullptr == pConstantName || '\0' == pConstantName[0])
		return nullptr;
	const uint64_t NameHash = Hash_VariableName(pConstantName);
	size_t Slot = static_cast<size_t>(NameHash) & pBindings->iVariableMask;
	VARIABLE_BINDING* pVariables = pBindings->Variables.data();
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
	VARIABLE_BINDING* pBinding = Find_Variable(pConstantName);
	if (nullptr == pBinding)
		return E_FAIL;

	const bool cacheable = pData && iLength > 0u && iLength <= sizeof(pBinding->LastValue);
	if (cacheable && pBinding->eLastValueKind == VARIABLE_BINDING::VALUE_KIND::RAW &&
		pBinding->iLastValueBytes == iLength &&
		0 == std::memcmp(pBinding->LastValue, pData, iLength))
		return S_OK;
	pBinding->eLastValueKind = VARIABLE_BINDING::VALUE_KIND::NONE;
	const HRESULT result = pBinding->pVariable->SetRawValue(pData, 0, iLength);
	if (SUCCEEDED(result)) pBinding->iRevision = ++m_pBindings->iRevision;
	if (SUCCEEDED(result) && cacheable)
	{
		std::memcpy(pBinding->LastValue, pData, iLength);
		pBinding->iLastValueBytes = iLength;
		pBinding->eLastValueKind = VARIABLE_BINDING::VALUE_KIND::RAW;
	}
	return result;
}

HRESULT CShader::Bind_Matrix(const char_t* pConstantName, const float4x4_t* pMatrix)
{
	VARIABLE_BINDING* pBinding = Find_Variable(pConstantName);
	if (nullptr == pBinding || nullptr == pBinding->pMatrix)
		return E_FAIL;

	if (pMatrix && pBinding->eLastValueKind == VARIABLE_BINDING::VALUE_KIND::MATRIX &&
		0 == std::memcmp(pBinding->LastValue, pMatrix, sizeof(*pMatrix)))
		return S_OK;
	pBinding->eLastValueKind = VARIABLE_BINDING::VALUE_KIND::NONE;
	const HRESULT result = pBinding->pMatrix->SetMatrix(reinterpret_cast<const float_t*>(pMatrix));
	if (SUCCEEDED(result)) pBinding->iRevision = ++m_pBindings->iRevision;
	if (SUCCEEDED(result) && pMatrix)
	{
		std::memcpy(pBinding->LastValue, pMatrix, sizeof(*pMatrix));
		pBinding->iLastValueBytes = sizeof(*pMatrix);
		pBinding->eLastValueKind = VARIABLE_BINDING::VALUE_KIND::MATRIX;
	}
	return result;
}

HRESULT CShader::Bind_Matrices(const char_t* pConstantName, const float4x4_t* pMatrices, uint32_t iNumMatrices)
{
	VARIABLE_BINDING* pBinding = Find_Variable(pConstantName);
	if (nullptr == pBinding || nullptr == pBinding->pMatrix)
		return E_FAIL;

	pBinding->eLastValueKind = VARIABLE_BINDING::VALUE_KIND::NONE;
	const HRESULT result = pBinding->pMatrix->SetMatrixArray(reinterpret_cast<const float_t*>(pMatrices), 0, iNumMatrices);
	if (SUCCEEDED(result)) pBinding->iRevision = ++m_pBindings->iRevision;
	return result;
}

HRESULT CShader::Bind_Texture(const char_t* pConstantName, const ComPtr<ID3D11ShaderResourceView>& pSRV)
{
	VARIABLE_BINDING* pBinding = Find_Variable(pConstantName);
	if (nullptr == pBinding || nullptr == pBinding->pResource)
		return E_FAIL;

	if (pBinding->bHasLastResource && pBinding->pLastResource == pSRV.Get())
		return S_OK;
	pBinding->bHasLastResource = false;
	const HRESULT result = pBinding->pResource->SetResource(pSRV.Get());
	if (SUCCEEDED(result)) pBinding->iRevision = ++m_pBindings->iRevision;
	if (SUCCEEDED(result))
	{
		// The Effect owns the SRV reference until another resource setter replaces it.
		pBinding->pLastResource = pSRV.Get();
		pBinding->bHasLastResource = true;
	}
	return result;
}

HRESULT CShader::Bind_Textures(const char_t* pConstantName, ID3D11ShaderResourceView** ppSRV, uint32_t iNumSRVs)
{
	VARIABLE_BINDING* pBinding = Find_Variable(pConstantName);
	if (nullptr == pBinding || nullptr == pBinding->pResource)
		return E_FAIL;

	pBinding->bHasLastResource = false;
	const HRESULT result = pBinding->pResource->SetResourceArray(ppSRV, 0, iNumSRVs);
	if (SUCCEEDED(result)) pBinding->iRevision = ++m_pBindings->iRevision;
	return result;
}



HRESULT CShader::Begin(uint32_t iPassIndex)
{
	if (nullptr == m_pBindings || iPassIndex >= m_iNumPasses)
		return E_FAIL;

	// Every HDR contributor uses the same current extraction curve. The effect
	// owner alone controls g_fEffectBloomIntensity, so concurrent skills stay independent.
	if (CRenderOutputContract::Get_Active() != RENDER_OUTPUT_CONTRACT::NONE &&
		nullptr != Find_Variable("g_fEffectBloomThreshold"))
	{
		const auto& quality = CGameInstance::Get().Get_RenderQualitySettings();
		if (FAILED(Bind_RawValue("g_fSceneBloomIntensity",
			&quality.fBloomIntensity, sizeof(quality.fBloomIntensity))) ||
			FAILED(Bind_RawValue("g_fEffectBloomThreshold",
			&quality.fBloomThreshold, sizeof(quality.fBloomThreshold))) ||
			FAILED(Bind_RawValue("g_fEffectBloomSoftKnee",
				&quality.fBloomSoftKnee, sizeof(quality.fBloomSoftKnee))))
			return E_FAIL;
	}
	if (m_pProgramVariants)
	{
		uint32_t program = 0u;
		if (FAILED(m_pProgramVariants->pProgram->pVariable->GetRawValue(&program, 0u, sizeof(program)))) return E_FAIL;
		// Forward-only map/vehicle programs retain their existing independent evaluator.
		if (program != 0u && !(program >= 33u && program <= 65u))
			return Apply_ProgramVariant(program, iPassIndex);
	}
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
