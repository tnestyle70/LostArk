#pragma once

// 장치/context 참조와 Prototype/Clone 수명 규칙을 상속한다.
#include "Component.h"

// Effect별 공유 캐시, 소유 문자열과 가변 pass/변수 배열을 선언한다.
// Clone과 프로그램 그룹이 Effect 바인딩의 공유 소유권을 유지한다.
#include <memory>
// Effect에서 찾은 변수 이름을 캐시 원소가 소유한다.
#include <string>
// pass·변수·품질 sampler와 입력 레이아웃의 가변 목록을 소유한다.
#include <vector>

// 공개 헤더는 Fx11 구현 헤더를 전파하지 않고 필요한 인터페이스 이름만 선언한다.
struct ID3DX11Effect;
struct ID3DX11EffectVariable;
struct ID3DX11EffectMatrixVariable;
struct ID3DX11EffectShaderResourceVariable;
struct ID3DX11EffectSamplerVariable;
struct ID3DX11EffectPass;

NS_BEGIN(Engine)

// 컴파일된 FX의 로드, 변수 바인딩과 draw 직전 pass 적용을 담당한다.
// Clone도 같은 Effect와 캐시를 공유하므로 독립 shader 상태가 아니며 동시 setter 호출을 내부에서 잠그지 않는다.
class ENGINE_DLL CShader final : public CComponent
{
private:
	CShader(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext);
public:
	// 기본 복사/대입은 GPU 자원과 Effect별 캐시의 소유 참조를 공유한다.
	CShader(const CShader& shader);
	CShader& operator=(const CShader& shader);
	virtual ~CShader();
public:
	// Create가 호출한다. EXE 옆 CSO와 입력 레이아웃·변수·variant를 임시 구성한 뒤 성공 시 멤버에 적용한다.
	virtual HRESULT Initialize_Prototype(const tchar_t* pShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* pElement, uint32_t iNumElements);
	// Clone의 초기화 훅이다. 현재 pArg를 소비하지 않고 공유된 FX 상태를 그대로 사용한다.
	virtual HRESULT Initialize(void* pArg) override;

public:
	// 렌더링 호출자가 이름으로 지정한 변수의 시작 위치에 iLength 바이트를 기록한다. 입력은 이 호출 동안 유효해야 한다.
	HRESULT Bind_RawValue(const char_t* pConstantName, const void* pData, uint32_t iLength);
	// 단일 행렬을 전달한다. 이 계층은 전치하지 않으므로 호출자가 shader 계약에 맞는 행렬을 준비한다.
	HRESULT Bind_Matrix(const char_t* pConstantName, const float4x4_t* pMatrix);
	// 본 행렬 같은 배열을 Effect에 설정하며 단일 행렬의 중복 입력 캐시를 무효화한다.
	HRESULT Bind_Matrices(const char_t* pConstantName, const float4x4_t* pMatrices, uint32_t iNumMatrices);
	// 단일 SRV를 설정한다. 성공 후 Effect가 참조를 소유하고 캐시는 동일 입력 비교용 차용 포인터만 갖는다.
	HRESULT Bind_Texture(const char_t* pConstantName, const ComPtr<ID3D11ShaderResourceView>& pSRV);
	// SRV 배열을 설정한다. Effect의 참조 취득 뒤에도 호출자의 배열 자체를 보관하지 않는다.
	HRESULT Bind_Textures(const char_t* pConstantName, ID3D11ShaderResourceView** ppSRV, uint32_t iNumSRVs);
	// 선택한 pass의 품질 상태·프로그램 variant를 반영하고 context에 입력 레이아웃과 pass를 적용한다. draw 호출은 소비자가 수행한다.
	HRESULT Begin(uint32_t iPassIndex);

private:
	// Effect 내부 인터페이스는 차용한다. 이름·입력 캐시와 revision은 EFFECT_BINDINGS가 소유한다.
	struct VARIABLE_BINDING final
	{
		std::string strName;
		uint64_t iNameHash = 0u;
		ID3DX11EffectVariable* pVariable = nullptr;
		ID3DX11EffectMatrixVariable* pMatrix = nullptr;
		ID3DX11EffectShaderResourceVariable* pResource = nullptr;
		enum class VALUE_KIND : uint8_t
		{
			NONE,
			RAW,
			MATRIX
		};
		// Effect가 공유하는 마지막 성공 입력이다. 어느 Clone이 설정해도 같은 캐시를 갱신한다.
		uint8_t LastValue[sizeof(float4x4_t)]{};
		uint32_t iLastValueBytes = 0u;
		VALUE_KIND eLastValueKind = VALUE_KIND::NONE;
		ID3D11ShaderResourceView* pLastResource = nullptr;
		bool bHasLastResource = false;
		uint64_t iRevision = 1u;
	};
	// pass annotation으로 프로그램과 무관한 처리를 기본 FX에 남기고 variant 사용 가능 여부를 구분한다.
	enum class PROGRAM_PASS_POLICY : uint8_t
	{
		VARIANT = 0u,
		BASE = 1u,
		UNAVAILABLE = 2u
	};
	struct TEXTURE_QUALITY_SAMPLER final
	{
		ID3DX11EffectSamplerVariable* pVariable = nullptr;
		ComPtr<ID3D11SamplerState> Levels[4]; // 원본 상태와 MinLOD 1/2/3 후보를 차례로 소유한다.
	};
	struct EFFECT_BINDINGS final
	{
		// 등록 변수 수의 두 배 이상인 테이블에서 선형 탐색한다. 빈 슬롯이 탐색의 끝이며,
		// 이름 문자열은 각 원소가 소유해 draw마다 Effect 이름 조회를 반복하지 않는다.
		std::vector<VARIABLE_BINDING> Variables;
		size_t iVariableMask = 0u;
		std::vector<ID3DX11EffectPass*> Passes;
		std::vector<PROGRAM_PASS_POLICY> PassPolicies;
		// 모든 컴포넌트 Clone이 Effect와 함께 sampler 상태도 공유한다.
		std::vector<TEXTURE_QUALITY_SAMPLER> TextureQualitySamplers;
		uint32_t iAppliedTextureMinMip = 0u;
		uint64_t iRevision = 1u;
	};
	struct PROGRAM_VARIANTS;
	// 지정한 기본 FX에서만 프로그램 그룹을 준비한다. 검증을 모두 통과해야 출력 pVariants를 교체한다.
	HRESULT Stage_ProgramVariants(const tchar_t* pShaderFilePath,
		const D3D11_INPUT_ELEMENT_DESC* pElements, uint32_t iNumElements,
		const std::shared_ptr<EFFECT_BINDINGS>& pBindings,
		std::shared_ptr<PROGRAM_VARIANTS>& pVariants);
	// Begin이 선택한 프로그램 그룹에 변경된 상수/SRV를 복사하고 해당 그룹의 Begin으로 연결한다.
	HRESULT Apply_ProgramVariant(uint32_t iProgram, uint32_t iPassIndex);
	// 대상 surface sampler의 원본과 MinLOD 후보를 로드 단계에서 준비한다. 저작 품질 설정은 저장하지 않는다.
	HRESULT Stage_TextureQualitySamplers(ID3DX11Effect* pEffect, EFFECT_BINDINGS& bindings);
	// 선택한 MinMip을 공유 Effect에 적용한다. 중간 실패 시 이미 건드린 sampler를 이전 상태로 돌리는 시도를 한다.
	HRESULT Apply_TextureQuality(uint32_t iMinMip);

	// Effect 소유 테이블에서 이름을 찾는다. 반환 포인터는 테이블이 살아 있는 동안만 차용할 수 있다.
	VARIABLE_BINDING* Find_Variable(const char_t* pConstantName) const;
	// 같은 이름 바이트가 같은 슬롯 탐색 시작점을 갖도록 계산한다. 충돌은 문자열 비교로 구분한다.
	static uint64_t Hash_VariableName(const char_t* pConstantName) noexcept;

	ComPtr<ID3DX11Effect>				m_pEffect = { nullptr };
	vector<ComPtr<ID3D11InputLayout>>	m_InputLayouts;
	// 변수 핸들과 마지막 성공 입력은 m_pEffect와 수명을 공유한다.
	// 변수 갱신은 CShader를 통해 처리하고, Begin은 선택한 pass를 매번 적용한다.
	std::shared_ptr<EFFECT_BINDINGS> m_pBindings;
	// Clone은 같은 FX 상태와 revision을 공유한다. variant 그룹에는 변경된 변수만 복사한다.
	std::shared_ptr<PROGRAM_VARIANTS> m_pProgramVariants;

private:
	uint32_t			m_iNumPasses = {};

public:
	// Prototype 등록용 소유 객체를 만든다. 초기화가 실패하면 임시 GPU 참조를 정리하고 nullptr를 반환한다.
	static unique_ptr<CShader> Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext,
		const tchar_t* pShaderFilePath, const D3D11_INPUT_ELEMENT_DESC* pElement, uint32_t iNumElements);
	// 기존 Effect를 공유하는 컴포넌트를 만든다. Clone에서 별도 CSO 로드는 수행하지 않는다.
	virtual shared_ptr<CPrototype> Clone(void* pArg) override;

};

NS_END
