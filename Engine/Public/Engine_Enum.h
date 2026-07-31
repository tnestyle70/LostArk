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

	enum class DEFERRED { DEBUG, DIRECTIONAL, POINT, COMBINED, FINAL, BLOOM_EXTRACT, BLOOM_BLUR_H, BLOOM_BLUR_V, END };

	//// Dynamic 컴포넌트 경우 매 프레임마다 갱신해야하는 컴포넌트 집단
	//enum COMPONENTID { ID_DYNAMIC, ID_STATIC, ID_END };



	//enum ROTATION { ROT_X, ROT_Y, ROT_Z, ROT_END };

	//enum TEXTUREID { TEX_NORMAL, TEX_CUBE, TEX_END };

	enum class LIGHT { DIRECTIONAL, POINT, END };

	enum class DIM { LB, RB, WHEEL, END };

	enum class DIMM { X, Y, WHEEL, END };

}
#endif // Engine_Enum_h__
