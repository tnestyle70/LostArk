#ifndef Engine_Typedef_h__
#define Engine_Typedef_h__

// 메모리 저장 형식과 SIMD 연산/인자 전달 형식의 DirectXMath 원본 타입을 사용한다.
#include <DirectXMath.h>
// 헤더 소비자가 공통으로 쓰는 고정 폭 정수 타입을 제공한다.
#include <cstdint>
// Windows 경로·원형 태그 등에 사용하는 소유형 wide 문자열을 별칭으로 공개한다.
#include <string>

namespace Engine
{
	typedef		bool						bool_t;

	// char_t와 tchar_t는 각각 char/wchar_t로 고정된다. 빌드 매크로에 따라 자동 전환하는 TCHAR와 구분한다.
	typedef		char						char_t;
	typedef		wchar_t						tchar_t;	
	typedef		std::wstring						wstring_t;

	typedef		float						f32_t;
	typedef		double						f64_t;

	/* 저장용 */
	// 객체 멤버·파일에서 읽은 값 등 메모리에 보관할 수학 값이다. SIMD 계산 전후에는 XMLoad/XMStore로 변환한다.
	typedef		DirectX::XMFLOAT2					float2_t;
	typedef		DirectX::XMFLOAT3					float3_t;
	typedef		DirectX::XMFLOAT4					float4_t;
	typedef		DirectX::XMFLOAT4X4					float4x4_t;

	/* 연산용 */
	// SIMD 계산 값과 함수 인자 전달용 별칭이다. F/G/H/C 접두 형식은 DirectXMath의 플랫폼별 호출 규약을 그대로 따른다.
	typedef		DirectX::XMVECTOR					vector_t;
	typedef		DirectX::FXMVECTOR					fvector_t;
	typedef		DirectX::GXMVECTOR					gvector_t;
	typedef		DirectX::HXMVECTOR					hvector_t;
	typedef		DirectX::CXMVECTOR					cvector_t;

	// 행렬 계산 값과 첫 행렬 인자 형식이다. float4x4_t의 저장 레이아웃과 역할을 구분한다.
	typedef		DirectX::XMMATRIX					matrix_t;
	typedef		DirectX::FXMMATRIX					fmatrix_t;


}

#endif // Engine_Typedef_h__
