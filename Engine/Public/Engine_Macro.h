#ifndef Engine_Macro_h__
#define Engine_Macro_h__

// enum을 정수 인덱스로 바꾼다. 유효 범위 검사는 하지 않으므로 END 등 잘못된 값은 호출자가 거른다.
#define			ETOI(_enum)			static_cast<int32_t>(_enum)
#define			ETOUI(_enum)		static_cast<uint32_t>(_enum)

// Engine DLL을 만들 때 내보내고 Client/도구에서 헤더를 사용할 때 가져오는 동일한 공개 경계다.
#ifdef	ENGINE_EXPORTS
#define ENGINE_DLL		_declspec(dllexport)
#else
#define ENGINE_DLL		_declspec(dllimport)
#endif

namespace Engine
{
	// 콘솔 검증 경로의 비대화식 오류 표시 정책을 설정/조회한다. 메시지 표시 자체는 Show_EngineMessage에 모은다.
	ENGINE_DLL void Set_NonInteractiveErrorMode(bool isEnabled);
	ENGINE_DLL bool Is_NonInteractiveErrorMode();
	ENGINE_DLL int Show_EngineMessage(const wchar_t* pMessage);
}

// 기존 문자열 리터럴을 엔진 오류 표시 함수로 연결한다. UI 실행 여부는 표시 함수의 현재 모드가 결정한다.
#ifndef			MSG_BOX
#define			MSG_BOX(_message)	Engine::Show_EngineMessage(TEXT(_message))
#endif

// 기존 헤더의 namespace 선언/사용 표기를 유지하는 매크로다.
#define			NS_BEGIN(_namespace)	namespace _namespace {
#define			NS_END					}
	
#define			USING(_namespace)		using namespace _namespace;
	
// 아래 검사는 호출한 함수에서 직접 반환한다. 소유 자원 정리와 상태 복원까지 대신 수행하지 않는다.
// MSG만 붙은 NULL_CHECK_MSG는 메시지를 표시할 뿐 반환하지 않는다는 차이에 주의한다.
#define NULL_CHECK( _ptr)	\
	{if( _ptr == 0){ return;}}
	
#define NULL_CHECK_RETURN( _ptr, _return)	\
	{if( _ptr == 0){return _return;}}
	
#define NULL_CHECK_MSG( _ptr, _message )		\
	{if( _ptr == 0){Engine::Show_EngineMessage(_message);}}
	
#define NULL_CHECK_RETURN_MSG( _ptr, _return, _message )	\
	{if( _ptr == 0){Engine::Show_EngineMessage(_message);return _return;}}
	
// HRESULT의 음수 실패를 검사해 호출 함수에서 반환한다. 반환값 지정 여부에 따라 원래 실패 코드가 E_FAIL 등으로 바뀔 수 있다.
#define FAILED_CHECK(_hr)	if( ((HRESULT)(_hr)) < 0 )	\
	{ Engine::Show_EngineMessage(L"Failed");  return E_FAIL;}
	
#define FAILED_CHECK_RETURN(_hr, _return)	if( ((HRESULT)(_hr)) < 0 )		\
	{ Engine::Show_EngineMessage(L"Failed");  return _return;}
	
#define FAILED_CHECK_MSG( _hr, _message)	if( ((HRESULT)(_hr)) < 0 )	\
	{ Engine::Show_EngineMessage(_message); return E_FAIL;}
	
#define FAILED_CHECK_RETURN_MSG( _hr, _return, _message)	if( ((HRESULT)(_hr)) < 0 )	\
	{ Engine::Show_EngineMessage(_message); return _return;}
	
	
	
// 소유 객체가 값 복사로 중복 소유되지 않도록 복사 생성과 대입을 삭제한다.
#define NO_COPY(CLASSNAME)										\
		private:												\
		CLASSNAME(const CLASSNAME&) = delete;					\
		CLASSNAME& operator = (const CLASSNAME&) = delete;

// Get의 함수 정적 객체 한 개를 참조로 공유한다. 정적 초기화 보장과 개별 멤버 함수의 스레드 안전성은 별개다.
#define DECLARE_SINGLETON(CLASSNAME)							\
		NO_COPY(CLASSNAME)										\
		public:													\
		static CLASSNAME& Get( void )							\
		{														\
			static CLASSNAME Instance{};						\
			return Instance;									\
		}

#endif // Engine_Macro_h__
