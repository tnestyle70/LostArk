// Effect와 공유 바인딩 상태를 소유하는 CShader 구현이다.
#include "Shader.h"
// variant가 처리할 프로그램 ID 범위와 지원 여부의 정본을 가져온다.
#include "SourceCharacterProgramRegistry.h"
//렌더링 품질 설정 구조체를 사용한다.
#include "Engine_RenderTypes.h"
// CSO에서 Effect 생성, 변수 접근과 pass 적용에 필요한 Fx11 구현 인터페이스다.
#include "Fx11/d3dx11effect.h"
// 현재 품질 설정과 profiler를 조회한다. 품질의 저작값을 여기서 변경하거나 저장하지 않는다.
#include "GameInstance.h"
// CSO 읽기·Effect 생성·바인딩·레이아웃 준비의 CPU 구간을 측정한다.
#include "Profiler.h"
// 현재 HDR 출력 참여 여부를 확인하여 bloom 공통 상수를 연결한다.
#include "Render_OutputContract.h"

// 경로/진단 문자열, signature·입력 바이트 비교와 할당 실패 처리를 사용한다.
// 진단 HRESULT와 프로그램 그룹 suffix를 고정 폭 wide 문자열에 작성한다.
#include <cwchar>
// 변수 이름·입력 signature·마지막 설정 바이트를 비교하고 값 캐시를 복사한다.
#include <cstring>
// 명시적으로 처리하는 할당 실패를 E_OUTOFMEMORY로 변환한다.
#include <new>

namespace
{
	constexpr uint64_t MaxCompiledEffectBytes = 256ull * 1024ull * 1024ull;
	constexpr uint32_t MaxCompiledEffectPasses = 1024u;

	// 초기화 성공·실패 지점에서 논리 경로/실제 CSO/바이트 수/소요 ms를 debugger에 남긴다.
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

	// 논리 shader 경로의 파일 이름을 EXE 디렉터리의 .cso로 연결한다. 원본 HLSL을 실행 중 컴파일하지 않는다.
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

	// 1바이트 이상 256 MiB 이하의 CSO를 소유 byte 배열로 읽는다. 파일 핸들은 모든 명시적 반환 경로에서 닫는다.
	// 호출자는 HRESULT가 성공한 경우에만 Bytecode를 사용하며 iByteCount는 진단용으로 실패 때도 남을 수 있다.
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

// 기본 FX와 프로그램 그룹 FX 사이의 변수 대응 및 마지막 복사 revision을 공유 소유한다.
struct CShader::PROGRAM_VARIANTS final
{
	// source/destination은 각각 고정된 Effect 바인딩 테이블을 차용한다.
	// Resources는 GetResourceArray가 늘린 참조를 전송 직후 반환하는 임시 공간이다.
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

// 기반 컴포넌트에 장치/context 참조를 전달한다. 실제 FX 준비는 Initialize_Prototype에서 수행한다.
CShader::CShader(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext)
	: CComponent { pDevice, pContext }
{
}

// 기본 복사는 Effect, 입력 레이아웃과 모든 공유 캐시의 수명을 함께 늘린다.
CShader::CShader(const CShader& shader) = default;

CShader& CShader::operator=(const CShader& shader) = default;

// COM/스마트 포인터 멤버가 수명을 정리한다. 공유 Clone이 남으면 그 Effect도 유지된다.
CShader::~CShader()
{
}

// Create가 호출하는 준비/검증/교체 경계다. 임시 Effect와 캐시를 완성하기 전에는 기존 멤버를 교체하지 않는다.
HRESULT CShader::Initialize_Prototype(const tchar_t* pShaderFilePath,
	const D3D11_INPUT_ELEMENT_DESC* pElements, uint32_t iNumElements)
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

	// 실행 파일 기준으로 CSO를 읽고 Fx11 Effect를 만든다. 실패 단계와 경로는 같은 진단 경로로 보고한다.
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

	// 첫 technique과 pass 개수의 유효성을 확인한 뒤 공유 변수 테이블을 구성한다.
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

	// 이 Prototype의 입력 요소는 고정되어 있다. pass의 입력 signature 바이트가 같으면
	// 동일한 불변 D3D 입력 레이아웃을 공유할 수 있다.
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
		Bindings->PassPolicies.reserve(TechniqueDesc.Passes);
		// 해시 mask로 순환할 수 있도록 2의 거듭제곱 용량을 고른다. 등록 변수 수의 두 배 이상을 확보해 빈 슬롯을 남긴다.
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

			// Fx11의 이름 조회 우선순위를 포함해 공개 이름을 한 번 해석한다.
			// 이후 렌더링은 이 Effect별 고정 테이블에서 변수 핸들을 찾는다.
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

			// annotation이 있으면 기본 FX 전용/variant/사용 불가 중 하나로 검증한다.
			PROGRAM_PASS_POLICY policy = PROGRAM_PASS_POLICY::VARIANT;
			auto* annotation = pPass->GetAnnotationByName("ProgramVariantPass");
			if (annotation && annotation->IsValid())
			{
				D3DX11_EFFECT_TYPE_DESC type{};
				int value = -1;
				auto* scalar = annotation->AsScalar();
				if (FAILED(annotation->GetType()->GetDesc(&type)) ||
					type.Class != D3D_SVC_SCALAR || type.Type != D3D_SVT_INT ||
					type.Elements != 0u || type.Rows != 1u || type.Columns != 1u ||
					nullptr == scalar || !scalar->IsValid() || FAILED(scalar->GetInt(&value)) ||
					value < 0 || value > static_cast<int>(PROGRAM_PASS_POLICY::UNAVAILABLE))
					return Fail(L"pass-program-policy", E_FAIL);
				policy = static_cast<PROGRAM_PASS_POLICY>(value);
			}
			Bindings->PassPolicies.push_back(policy);

			// 앞서 만든 pass와 입력 signature가 byte 단위로 같으면 입력 레이아웃의 COM 참조를 공유한다.
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

	// 입력 레이아웃 검증 이후 품질 sampler와 프로그램 그룹까지 준비해야 적용 가능한 Prototype이 된다.
	Result = Stage_TextureQualitySamplers(Effect.Get(), *Bindings);
	if (FAILED(Result))
		return Fail(L"texture-quality-samplers", Result);

	std::shared_ptr<PROGRAM_VARIANTS> ProgramVariants;
	Result = Stage_ProgramVariants(pShaderFilePath, pElements, iNumElements, Bindings, ProgramVariants);
	if (FAILED(Result))
		return Fail(L"program-variants", Result);

	// 검증이 끝난 소유 자원을 한 객체에 연결한다. 이전 멤버는 이 지점 전의 실패에서는 보존된다.
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

// Clone 초기화 훅이다. 현재 복사된 Effect 공유 상태 외의 인스턴스 입력은 없다.
HRESULT CShader::Initialize(void* pArg)
{
	return S_OK;
}

// 초기화가 호출한다. 지정한 surface sampler의 기존 MinLOD/MaxLOD 범위 안에서 품질 후보를 준비한다.
HRESULT CShader::Stage_TextureQualitySamplers(ID3DX11Effect* pEffect, EFFECT_BINDINGS& bindings)
{
	// 기존 shader 계약에서 surface 입력으로 지정한 sampler만 품질 후보를 만든다.
	// BRDF 표나 roughness cube를 읽는 lightmap/lookup sampler의 상태는 유지한다.
	constexpr const char* names[] = {
		"MaterialAnisotropicSampler", "SurfaceAnisotropicSampler",
		"SurfaceMirrorUSampler", "SurfaceMirrorVSampler", "SurfaceMirrorUVSampler",
		"SourceCharacterSampler", "SourceCharacterStampSampler",
		"SourceMapMonsterStateSampler", "SourceMapSkyCloudSampler", "LinearSampler"
	};
	auto* const surfaceVariable = pEffect->GetVariableByName("SurfaceAnisotropicSampler");
	const bool surface = surfaceVariable && surfaceVariable->IsValid();
	try
	{
		for (const char* name : names)
		{
			// 인스턴싱 surface pass만 LinearSampler를 이 품질 계약에 포함한다.
			// 같은 이름을 쓰는 UI와 deferred 후처리는 자체 sampler 상태를 유지한다.
			if (0 == std::strcmp(name, "LinearSampler") && !surface)
				continue;
			auto* variable = pEffect->GetVariableByName(name);
			if (!variable || !variable->IsValid())
				continue;
			auto* sampler = variable->AsSampler();
			D3DX11_EFFECT_TYPE_DESC type{};
			if (!sampler || !sampler->IsValid() || FAILED(variable->GetType()->GetDesc(&type)) ||
				type.Elements != 0u)
				return E_FAIL;
			TEXTURE_QUALITY_SAMPLER staged;
			staged.pVariable = sampler;
			HRESULT result = sampler->GetSampler(0u, staged.Levels[0].GetAddressOf());
			if (FAILED(result) || !staged.Levels[0])
				return FAILED(result) ? result : E_FAIL;
			D3D11_SAMPLER_DESC original{};
			staged.Levels[0]->GetDesc(&original);
			for (uint32_t mip = 1u; mip < 4u; ++mip)
			{
				auto desc = original;
				desc.MinLOD = std::min(desc.MaxLOD, std::max(desc.MinLOD, static_cast<float>(mip)));
				result = m_pDevice->CreateSamplerState(&desc, staged.Levels[mip].GetAddressOf());
				if (FAILED(result))
					return result;
			}
			bindings.TextureQualitySamplers.push_back(std::move(staged));
		}
	}
	catch (const std::bad_alloc&)
	{
		return E_OUTOFMEMORY;
	}
	return S_OK;
}

// Begin이 현재 품질을 공유 Effect에 적용한다. 이미 같은 단계면 생략하고 모든 setter 성공 후 적용 단계를 갱신한다.
HRESULT CShader::Apply_TextureQuality(uint32_t iMinMip)
{
	if (iMinMip >= 4u)
		return E_INVALIDARG;
	auto& bindings = *m_pBindings;
	if (bindings.iAppliedTextureMinMip == iMinMip)
		return S_OK;
	for (size_t i = 0u; i < bindings.TextureQualitySamplers.size(); ++i)
	{
		auto& sampler = bindings.TextureQualitySamplers[i];
		const HRESULT result = sampler.pVariable->SetSampler(0u, sampler.Levels[iMinMip].Get());
		if (FAILED(result))
		{
			// 아직 draw를 제출하지 않았으므로 건드린 sampler를 이전 공유 상태로 돌린다.
			// 복구 SetSampler의 결과는 별도로 전파하지 않고 최초 실패를 반환한다.
			for (size_t rollback = 0u; rollback <= i; ++rollback)
			{
				auto& previous = bindings.TextureQualitySamplers[rollback];
				previous.pVariable->SetSampler(0u, previous.Levels[bindings.iAppliedTextureMinMip].Get());
			}
			return result;
		}
	}
	bindings.iAppliedTextureMinMip = iMinMip;
	return S_OK;
}

// 지정한 기본 shader만 그룹별 CSO를 추가로 로드한다. 그룹 shader는 이 단계에서 다시 기본 그룹으로 확장되지 않는다.
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
			if (variable.strName == "g_SourceCharacterProgram")
				staged->pProgram = &variable;
		if (nullptr == staged->pProgram)
			return E_FAIL;
		const auto& ranges = SourceCharacterProgramGroups;
		staged->Groups.reserve(std::size(ranges));
		for (const auto& range : ranges)
		{
			wchar_t suffix[40]{};
			swprintf_s(suffix, L"_SourceGroup%03u.hlsl", range.first);
			const std::wstring variantPath = path.substr(0u, path.size() - 5u) + suffix;
			PROGRAM_VARIANTS::VARIANT variant;
			variant.iFirstProgram = range.first;
			variant.iLastProgram = range.last;
			variant.pShader = CShader::Create(m_pDevice, m_pContext, variantPath.c_str(), pElements, iNumElements);
			if (!variant.pShader || variant.pShader->m_iNumPasses != pBindings->Passes.size())
				return E_FAIL;
			// 기본/그룹 FX의 pass 이름·입력 signature·사용 정책이 같아야 같은 pass 번호로 전환할 수 있다.
			for (size_t pass = 0u; pass < pBindings->Passes.size(); ++pass)
			{
				D3DX11_PASS_DESC source{},
					destination{};
				if (FAILED(pBindings->Passes[pass]->GetDesc(&source)) ||
					FAILED(variant.pShader->m_pBindings->Passes[pass]->GetDesc(&destination)) ||
					nullptr == source.Name || nullptr == destination.Name ||
					0 != std::strcmp(source.Name, destination.Name) ||
					nullptr == source.pIAInputSignature || nullptr == destination.pIAInputSignature ||
					0u == source.IAInputSignatureSize || source.IAInputSignatureSize != destination.IAInputSignatureSize ||
					0 != std::memcmp(source.pIAInputSignature, destination.pIAInputSignature, source.IAInputSignatureSize))
					return E_FAIL;
				const auto sourcePolicy = pBindings->PassPolicies[pass];
				const auto destinationPolicy = variant.pShader->m_pBindings->PassPolicies[pass];
				if ((sourcePolicy == PROGRAM_PASS_POLICY::BASE && destinationPolicy != PROGRAM_PASS_POLICY::UNAVAILABLE) ||
					(sourcePolicy == PROGRAM_PASS_POLICY::VARIANT && destinationPolicy != PROGRAM_PASS_POLICY::VARIANT) ||
					sourcePolicy == PROGRAM_PASS_POLICY::UNAVAILABLE)
					return E_FAIL;
			}
			// 전송 가능한 변수의 타입·배열 크기를 대조해 복사 경로를 고정한다. 불일치하면 전체 후보를 버린다.
			variant.Variables.reserve(pBindings->Variables.size() / 2u);
			for (auto& source : pBindings->Variables)
			{
				if (nullptr == source.pVariable)
					continue;
				D3DX11_EFFECT_TYPE_DESC sourceType{};
				if (FAILED(source.pVariable->GetType()->GetDesc(&sourceType)))
					return E_FAIL;
				// shader와 상태 객체는 컴파일된 각 variant가 소유한다.
				// 호출자가 조절하는 상수와 shader resource만 다른 FX로 복사한다.
				if (nullptr == source.pResource && sourceType.Class == D3D_SVC_OBJECT)
					continue;
				auto* destination = variant.pShader->Find_Variable(source.strName.c_str());
				D3DX11_EFFECT_TYPE_DESC destinationType{};
				if (nullptr == destination || FAILED(destination->pVariable->GetType()->GetDesc(&destinationType)) ||
					sourceType.Class != destinationType.Class || sourceType.Type != destinationType.Type ||
					sourceType.Elements != destinationType.Elements || sourceType.Rows != destinationType.Rows ||
					sourceType.Columns != destinationType.Columns || sourceType.Members != destinationType.Members ||
					sourceType.UnpackedSize != destinationType.UnpackedSize ||
					(nullptr != source.pResource) != (nullptr != destination->pResource))
					return E_FAIL;
				PROGRAM_VARIANTS::VARIABLE_COPY copy;
				copy.pSource = &source;
				copy.pDestination = destination;
				if (source.pResource)
					copy.Resources.resize(std::max(1u, sourceType.Elements));
				else
					copy.Bytes.resize(sourceType.UnpackedSize);
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

// Begin이 선택한 그룹에 이전 복사 이후 바뀐 변수만 동기화한다. 실패하면 draw 전 실패를 반환하며 이미 복사한 변수는 남을 수 있다.
HRESULT CShader::Apply_ProgramVariant(uint32_t iProgram, uint32_t iPassIndex)
{
	if (!Is_SourceCharacterProgramSupported(iProgram))
		return E_INVALIDARG;
	for (auto& variant : m_pProgramVariants->Groups)
	{
		if (iProgram < variant.iFirstProgram || iProgram > variant.iLastProgram)
			continue;
		if (variant.iCopiedRevision == m_pBindings->iRevision)
			return variant.pShader->Begin(iPassIndex);
		auto* copies = variant.Variables.data();
		for (size_t variableIndex = 0u; variableIndex < variant.Variables.size(); ++variableIndex)
		{
			auto& copy = copies[variableIndex];
			if (copy.iCopiedRevision == copy.pSource->iRevision)
				continue;
			HRESULT result = S_OK;
			// SRV는 Effect API가 취득한 참조를 전송 후 해제한다. 일반 상수는 준비한 byte 배열을 거쳐 복사한다.
			if (!copy.Resources.empty())
			{
				const uint32_t count = static_cast<uint32_t>(copy.Resources.size());
				result = copy.pSource->pResource->GetResourceArray(copy.Resources.data(), 0u, count);
				if (SUCCEEDED(result))
					result = copy.pDestination->pResource->SetResourceArray(copy.Resources.data(), 0u, count);
				for (auto*& resource : copy.Resources)
				{
					if (resource)
						resource->Release();
					resource = nullptr;
				}
				copy.pDestination->bHasLastResource = false;
			}
			else if (!copy.Bytes.empty())
			{
				const uint32_t count = static_cast<uint32_t>(copy.Bytes.size());
				result = copy.pSource->pVariable->GetRawValue(copy.Bytes.data(), 0u, count);
				if (SUCCEEDED(result))
					result = copy.pDestination->pVariable->SetRawValue(copy.Bytes.data(), 0u, count);
				copy.pDestination->eLastValueKind = VARIABLE_BINDING::VALUE_KIND::NONE;
			}
			if (FAILED(result))
				return result;
			copy.iCopiedRevision = copy.pSource->iRevision;
		}
		variant.iCopiedRevision = m_pBindings->iRevision;
		return variant.pShader->Begin(iPassIndex);
	}
	return E_INVALIDARG;
}

// 빈 슬롯을 만날 때까지 해시와 실제 이름을 함께 비교한다. 반환 핸들은 공유 Effect 수명에 종속된다.
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

// 이름의 unsigned byte에 FNV-1a 계산을 적용한다. nullptr/종료 문자 검증은 호출 경계에서 맡는다.
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

// 작은 단일 입력은 마지막 성공 바이트와 비교한다. setter 실패 시 캐시를 무효 상태로 두고 revision은 늘리지 않는다.
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
	if (SUCCEEDED(result))
		pBinding->iRevision = ++m_pBindings->iRevision;
	if (SUCCEEDED(result) && cacheable)
	{
		std::memcpy(pBinding->LastValue, pData, iLength);
		pBinding->iLastValueBytes = iLength;
		pBinding->eLastValueKind = VARIABLE_BINDING::VALUE_KIND::RAW;
	}
	return result;
}

// 같은 단일 행렬의 중복 setter를 줄인다. 입력 좌표계·전치는 호출자가 준비하며 성공 후에만 캐시를 저장한다.
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
	if (SUCCEEDED(result))
		pBinding->iRevision = ++m_pBindings->iRevision;
	if (SUCCEEDED(result) && pMatrix)
	{
		std::memcpy(pBinding->LastValue, pMatrix, sizeof(*pMatrix));
		pBinding->iLastValueBytes = sizeof(*pMatrix);
		pBinding->eLastValueKind = VARIABLE_BINDING::VALUE_KIND::MATRIX;
	}
	return result;
}

// 배열 설정은 단일 값 캐시를 무효화한다. 성공 revision을 통해 다음 프로그램 그룹 적용에도 전달한다.
HRESULT CShader::Bind_Matrices(const char_t* pConstantName, const float4x4_t* pMatrices, uint32_t iNumMatrices)
{
	VARIABLE_BINDING* pBinding = Find_Variable(pConstantName);
	if (nullptr == pBinding || nullptr == pBinding->pMatrix)
		return E_FAIL;

	pBinding->eLastValueKind = VARIABLE_BINDING::VALUE_KIND::NONE;
	const HRESULT result = pBinding->pMatrix->SetMatrixArray(reinterpret_cast<const float_t*>(pMatrices), 0, iNumMatrices);
	if (SUCCEEDED(result))
		pBinding->iRevision = ++m_pBindings->iRevision;
	return result;
}

// 단일 SRV의 동일 포인터를 비교한다. Effect setter 성공 후에만 차용 비교 포인터를 유효하게 만든다.
HRESULT CShader::Bind_Texture(const char_t* pConstantName, const ComPtr<ID3D11ShaderResourceView>& pSRV)
{
	VARIABLE_BINDING* pBinding = Find_Variable(pConstantName);
	if (nullptr == pBinding || nullptr == pBinding->pResource)
		return E_FAIL;

	if (pBinding->bHasLastResource && pBinding->pLastResource == pSRV.Get())
		return S_OK;
	pBinding->bHasLastResource = false;
	const HRESULT result = pBinding->pResource->SetResource(pSRV.Get());
	if (SUCCEEDED(result))
		pBinding->iRevision = ++m_pBindings->iRevision;
	if (SUCCEEDED(result))
	{
		// 다른 리소스 setter가 교체할 때까지 Effect가 SRV 참조를 소유한다.
		pBinding->pLastResource = pSRV.Get();
		pBinding->bHasLastResource = true;
	}
	return result;
}

// 배열 setter는 단일 SRV 캐시를 무효화하고 성공한 변경만 revision에 반영한다.
HRESULT CShader::Bind_Textures(const char_t* pConstantName, ID3D11ShaderResourceView** ppSRV, uint32_t iNumSRVs)
{
	VARIABLE_BINDING* pBinding = Find_Variable(pConstantName);
	if (nullptr == pBinding || nullptr == pBinding->pResource)
		return E_FAIL;

	pBinding->bHasLastResource = false;
	const HRESULT result = pBinding->pResource->SetResourceArray(ppSRV, 0, iNumSRVs);
	if (SUCCEEDED(result))
		pBinding->iRevision = ++m_pBindings->iRevision;
	return result;
}

// 렌더링 소비자가 draw 전에 호출한다. pass 정책→공통 상수→프로그램 그룹→품질 sampler→context 적용 순서를 유지한다.
HRESULT CShader::Begin(uint32_t iPassIndex)
{
	if (nullptr == m_pBindings || iPassIndex >= m_iNumPasses)
		return E_FAIL;
	const auto passPolicy = m_pBindings->PassPolicies[iPassIndex];
	// 직접 로드한 프로그램 그룹에서 native PS가 제외된 기본 FX 전용 pass는
	// 사용할 수 없으므로 성공으로 처리하지 않는다.
	if (passPolicy == PROGRAM_PASS_POLICY::UNAVAILABLE)
		return E_FAIL;

	// HDR 출력은 현재 장면의 bloom 추출 곡선을 공유한다. g_fEffectBloomIntensity는
	// 개별 이펙트 소유자가 설정하도록 두어 다른 스킬의 강도 값에 개입하지 않는다.
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
	if (m_pProgramVariants && passPolicy != PROGRAM_PASS_POLICY::BASE)
	{
		uint32_t program = 0u;
		if (FAILED(m_pProgramVariants->pProgram->pVariable->GetRawValue(&program, 0u, sizeof(program))))
			return E_FAIL;
		// forward 전용 맵/탈것 프로그램은 기존의 독립 평가 경로를 유지한다.
		if (program != 0u && !(program >= 33u && program <= 65u) && program != 209u)
			return Apply_ProgramVariant(program, iPassIndex);
	}
	if (!m_pBindings->TextureQualitySamplers.empty())
	{
		uint32_t minMip = CGameInstance::Get().Get_TextureMinMip();
		if (minMip != 0u)
		{
			D3DX11_PASS_DESC pass{};
			if (FAILED(m_pBindings->Passes[iPassIndex]->GetDesc(&pass)))
				return E_FAIL;
			// 모델 이펙트는 재질 surface와 FX/LinearSampler를 공유하지만 native pass에서는
			// 기존 wrap/clamp/장면 읽기 계약을 함께 유지한다. 일반 재질과 masked shadow는
			// 선택한 surface 품질을 계속 사용한다.
			if (pass.Name && 0 == std::strncmp(pass.Name, "EffectModelCueNative", 20u))
				minMip = 0u;
		}
		const HRESULT qualityResult = Apply_TextureQuality(minMip);
		if (FAILED(qualityResult))
			return qualityResult;
	}
	m_pContext->IASetInputLayout(m_InputLayouts[iPassIndex].Get());

	return m_pBindings->Passes[iPassIndex]->Apply(0, m_pContext.Get());
}

// Prototype 등록이 사용할 소유 객체를 만든다. 초기화 실패는 로그를 남기고 임시 객체를 해제한다.
unique_ptr<CShader> CShader::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
	const tchar_t* pShaderFilePath,
	const D3D11_INPUT_ELEMENT_DESC* pElements, uint32_t iNumElements)
{
	auto pInstance = unique_ptr<CShader>(new CShader(pDevice, pContext));

	if (FAILED(pInstance->Initialize_Prototype(pShaderFilePath, pElements, iNumElements)))
	{
		OutputDebugStringA("[Engine][Shader] Create failed.\n");
		return nullptr;
	}

	return pInstance;
}

// 복사된 컴포넌트를 shared_ptr로 반환한다. Effect와 캐시는 원본 및 다른 Clone과 공유된다.
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
