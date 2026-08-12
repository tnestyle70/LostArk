#ifndef Engine_Macro_h__
#define Engine_Macro_h__

#define			ETOI(_enum)			static_cast<int32_t>(_enum)
#define			ETOUI(_enum)		static_cast<uint32_t>(_enum)

#ifdef	ENGINE_EXPORTS
#define ENGINE_DLL		_declspec(dllexport)
#else
#define ENGINE_DLL		_declspec(dllimport)
#endif

namespace Engine
{
	ENGINE_DLL void Set_NonInteractiveErrorMode(bool isEnabled);
	ENGINE_DLL bool Is_NonInteractiveErrorMode();
	ENGINE_DLL int Show_EngineMessage(const wchar_t* pMessage);
}

#ifndef			MSG_BOX
#define			MSG_BOX(_message)	Engine::Show_EngineMessage(TEXT(_message))
#endif

#define			NS_BEGIN(_namespace)	namespace _namespace {
#define			NS_END					}
	
#define			USING(_namespace)		using namespace _namespace;
	
#define NULL_CHECK( _ptr)	\
	{if( _ptr == 0){ return;}}
	
#define NULL_CHECK_RETURN( _ptr, _return)	\
	{if( _ptr == 0){return _return;}}
	
#define NULL_CHECK_MSG( _ptr, _message )		\
	{if( _ptr == 0){Engine::Show_EngineMessage(_message);}}
	
#define NULL_CHECK_RETURN_MSG( _ptr, _return, _message )	\
	{if( _ptr == 0){Engine::Show_EngineMessage(_message);return _return;}}
	
#define FAILED_CHECK(_hr)	if( ((HRESULT)(_hr)) < 0 )	\
	{ Engine::Show_EngineMessage(L"Failed");  return E_FAIL;}
	
#define FAILED_CHECK_RETURN(_hr, _return)	if( ((HRESULT)(_hr)) < 0 )		\
	{ Engine::Show_EngineMessage(L"Failed");  return _return;}
	
#define FAILED_CHECK_MSG( _hr, _message)	if( ((HRESULT)(_hr)) < 0 )	\
	{ Engine::Show_EngineMessage(_message); return E_FAIL;}
	
#define FAILED_CHECK_RETURN_MSG( _hr, _return, _message)	if( ((HRESULT)(_hr)) < 0 )	\
	{ Engine::Show_EngineMessage(_message); return _return;}
	
	
	
#define NO_COPY(CLASSNAME)										\
		private:												\
		CLASSNAME(const CLASSNAME&) = delete;					\
		CLASSNAME& operator = (const CLASSNAME&) = delete;

#define DECLARE_SINGLETON(CLASSNAME)							\
		NO_COPY(CLASSNAME)										\
		public:													\
		static CLASSNAME& Get( void )							\
		{														\
			static CLASSNAME Instance{};						\
			return Instance;									\
		}

#endif // Engine_Macro_h__
