#ifndef Engine_Define_h__
#define Engine_Define_h__

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <d3dcompiler.h>
#define DIRECTINPUT_VERSION	0x0800
#include <dinput.h>

#include "DirectXTK/DDSTextureLoader.h"
#include "DirectXTK/WICTextureLoader.h"
#include "DirectXTK/SpriteBatch.h"
#include "DirectXTK/SpriteFont.h"
#include "DirectXTK/ScreenGrab.h"
#include "DirectXTK/PrimitiveBatch.h"
#include "DirectXTK/Effects.h"
#include "DirectXTK/VertexTypes.h"

#include "Fx11/d3dx11effect.h"
#include "Assimp/scene.h"
#include "Assimp/Importer.hpp"
#include "Assimp/postprocess.h"

using namespace DirectX;

#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <functional>
#include <string>
#include <unordered_map>
#include <ctime>
#include <memory>
using namespace std;

#include <wrl/client.h>
using namespace Microsoft::WRL;

#include "Engine_Enum.h"
#include "Engine_Macro.h"
#include "Engine_Struct.h"
#include "Engine_Typedef.h"
#include "Engine_Function.h"

namespace Engine
{
	//const unsigned int g_iMaxWidth = 2560;
	//const unsigned int g_iMaxHeight = 1440;
	// const unsigned int g_iMaxWidth = 1280;
	// const unsigned int g_iMaxHeight = 720;

	/*
	const unsigned int g_iMaxWidth = 16384;
	const unsigned int g_iMaxHeight = 9216;
	*/
	const unsigned int g_iMaxWidth = 8192;
	const unsigned int g_iMaxHeight = 4608;

	static const wstring_t g_strTransformComTag = TEXT("Com_Transform");
}




#pragma warning(disable : 4251)

#ifdef _DEBUG

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifndef DBG_NEW

#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ ) 
#define new DBG_NEW 

#endif
#endif

using namespace std;
using namespace Engine;

#endif // Engine_Define_h__
