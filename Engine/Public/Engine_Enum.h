#ifndef Engine_Enum_h__
#define Engine_Enum_h__

namespace Engine
{
	enum class WINMODE { FULL, WIN, END };	

	enum class RENDERGROUP { PRIORITY, SHADOW, NONBLEND, NONLIGHT, BLEND, UI, END };
	
	enum class STATE { RIGHT, UP, LOOK, POSITION, END };

	enum class D3DTS { VIEW, PROJ, END };

	enum class MODEL { NONANIM, ANIM, END };

	enum class POINT { A, B, C, END };

	enum class LINE { AB, BC, CA, END };

	enum class COLLIDER { SPHERE, AABB, OBB, END };

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
		END
	};

	//// Dynamic 컴포넌트 경우 매 프레임마다 갱신해야하는 컴포넌트 집단
	//enum COMPONENTID { ID_DYNAMIC, ID_STATIC, ID_END };



	//enum ROTATION { ROT_X, ROT_Y, ROT_Z, ROT_END };

	//enum TEXTUREID { TEX_NORMAL, TEX_CUBE, TEX_END };

	enum class LIGHT { DIRECTIONAL, POINT, END };

	enum class DIM { LB, RB, WHEEL, END };

	enum class DIMM { X, Y, WHEEL, END };

}
#endif // Engine_Enum_h__
