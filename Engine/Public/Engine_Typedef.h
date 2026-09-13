#ifndef Engine_Typedef_h__
#define Engine_Typedef_h__

#include <DirectXMath.h>
#include <cstdint>
#include <string>

namespace Engine
{
	typedef		bool						bool_t;

	typedef		char						char_t;
	typedef		wchar_t						tchar_t;	
	typedef		std::wstring						wstring_t;

	typedef		float						f32_t;
	typedef		double						f64_t;

	/* 저장용 */
	typedef		DirectX::XMFLOAT2					float2_t;
	typedef		DirectX::XMFLOAT3					float3_t;
	typedef		DirectX::XMFLOAT4					float4_t;
	typedef		DirectX::XMFLOAT4X4					float4x4_t;

	/* 연산용 */
	typedef		DirectX::XMVECTOR					vector_t;
	typedef		DirectX::FXMVECTOR					fvector_t;
	typedef		DirectX::GXMVECTOR					gvector_t;
	typedef		DirectX::HXMVECTOR					hvector_t;
	typedef		DirectX::CXMVECTOR					cvector_t;

	typedef		DirectX::XMMATRIX					matrix_t;
	typedef		DirectX::FXMMATRIX					fmatrix_t;


}

#endif // Engine_Typedef_h__
