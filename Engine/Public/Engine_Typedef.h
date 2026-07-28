#ifndef Engine_Typedef_h__
#define Engine_Typedef_h__

namespace Engine
{
	typedef		bool						bool_t;

	typedef		char						char_t;
	typedef		wchar_t						tchar_t;	
	typedef		wstring						wstring_t;

	typedef		float						f32_t;
	typedef		double						f64_t;

	/* 저장용 */
	typedef		XMFLOAT2					float2_t;
	typedef		XMFLOAT3					float3_t;
	typedef		XMFLOAT4					float4_t;
	typedef		XMFLOAT4X4					float4x4_t;

	/* 연산용 */
	typedef		XMVECTOR					vector_t;
	typedef		FXMVECTOR					fvector_t;
	typedef		GXMVECTOR					gvector_t;
	typedef		HXMVECTOR					hvector_t;
	typedef		CXMVECTOR					cvector_t;

	typedef		XMMATRIX					matrix_t;
	typedef		FXMMATRIX					fmatrix_t;


}

#endif // Engine_Typedef_h__
