#ifndef Engine_Enum_h__
#define Engine_Enum_h__

namespace Engine
{
	// 창 생성/초기화에서 사용하는 모드다. FULL=0, WIN=1은 swap chain의 Windowed 변환과 연결된다.
	enum class WINMODE { FULL, WIN, END };	

	// Renderer가 제출 객체를 묶는 분류다. 열거 순서를 바꾸면 배열 인덱스도 바뀌며 실제 패스 호출 순서는 Renderer가 결정한다.
	enum class RENDERGROUP { PRIORITY, SHADOW, NONBLEND, DEFERRED_OVERLAY, NONLIGHT, BLEND, UI, SCENE_UI, PICKING, END };
	
	// Transform의 4x4 행렬에서 오른쪽·위·전방 basis와 위치 행을 선택한다.
	enum class STATE { RIGHT, UP, LOOK, POSITION, END };

	// PipeLine이 보관하는 카메라 뷰/투영 행렬의 배열 인덱스다. END는 유효 행렬이 아니다.
	enum class D3DTS { VIEW, PROJ, END };

	// 모델 준비 시 정적 메시와 골격 애니메이션 메시 경로를 구분한다.
	enum class MODEL { NONANIM, ANIM, END };

	// navigation 삼각형의 세 꼭짓점과 변을 동일한 순서로 참조한다.
	enum class POINT { A, B, C, END };

	enum class LINE { AB, BC, CA, END };

	// Collider가 생성할 경계 도형을 선택한다. 실제 교차 검사는 각 Bounding 구현이 담당한다.
	enum class COLLIDER { SPHERE, AABB, OBB, END };

	// Deferred FX의 실제 pass 번호다. 중간에 비어 있는 번호도 유지해야 CShader::Begin이 같은 pass를 선택한다.
	enum class DEFERRED
	{
		DEBUG = 0,
		DIRECTIONAL = 1,
		POINT = 2,
		COMBINED = 3,
		FINAL = 4,
		BLOOM_EXTRACT = 5,
		BLOOM_BLUR_H = 6,
		BLOOM_BLUR_V = 7,
		SCENE_RESOLVE = 8,
		PRESENTATION_RGB_NOISE = 9,
		PRESENTATION_ZOOM_BLUR = 10,
		PRESENTATION_FILM_NOISE = 11,
		SSAO_RAW = 12,
		SSAO_BLUR = 13,
		PRESENTATION_CHROMATIC_ABERRATION = 15,
		SPOT = 16,
		PRESENTATION_DISPLAY_OVERLAY = 17,
		SOURCE_LIGHT_MASK = 18,
		SOURCE_DIRECTIONAL = 19,
		SOURCE_POINT = 20,
		SOURCE_SPOT = 21,
		// 22~27은 기존에 추가된 인스턴싱 광원 pass가 사용한다.
		SOURCE_LUT_BAKE = 28,
		SOURCE_LUT_BAKE_NEUTRAL = 29,
		PORTRAIT_RESOLVE = 30,
		END
	};

	//// Dynamic ������Ʈ ��� �� �����Ӹ��� �����ؾ��ϴ� ������Ʈ ����
	//enum COMPONENTID { ID_DYNAMIC, ID_STATIC, ID_END };



	//enum ROTATION { ROT_X, ROT_Y, ROT_Z, ROT_END };

	//enum TEXTUREID { TEX_NORMAL, TEX_CUBE, TEX_END };

	// Light_Manager와 셰이더의 광원 처리 분기를 선택한다. 방향광·점광원·스폿광은 입력 범위가 다르다.
	enum class LIGHT { DIRECTIONAL, POINT, SPOT, END };

	// Input_Device의 버튼 상태 인덱스다. 아래 DIMM은 버튼 상태와 별개인 이동량 축을 가리킨다.
	enum class DIM { LB, RB, WHEEL, END };

	enum class DIMM { X, Y, WHEEL, END };

}
#endif // Engine_Enum_h__
