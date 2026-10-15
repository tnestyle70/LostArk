#ifndef Engine_Function_h__
#define Engine_Function_h__

// 참조 카운트 반환에 사용하는 uint32_t를 공통 타입 경로로 가져온다.
#include "Engine_Typedef.h"

namespace Engine
{
	// 템플릿은 기능의 정해져있으나 자료형은 정해져있지 않은 것
	// 기능을 인스턴스화 하기 위하여 만들어두는 틀

	// 단일 new 객체의 소유 포인터를 지운 뒤 호출자의 변수도 nullptr로 만든다. 다른 별칭 포인터는 갱신하지 않는다.
	template<typename T>
	void	Safe_Delete(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete Pointer;
			Pointer = nullptr;
		}
	}

	// new[]로 만든 배열에만 사용한다. 소유 형태가 단일 new인 객체와 혼용하면 안 된다.
	template<typename T>
	void	Safe_Delete_Array(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete [] Pointer;
			Pointer = nullptr;
		}
	}

	// COM 또는 같은 참조 카운트 API의 강한 참조 하나를 추가한다. 입력이 nullptr이면 0이다.
	template<typename T>
	uint32_t Safe_AddRef(T& Instance)
	{
		uint32_t		iRefCnt = 0;

		if (nullptr != Instance)
			iRefCnt = Instance->AddRef();

		return iRefCnt;
	}

	// 강한 참조 하나를 반환하고 남은 카운트를 돌려준다. 0일 때만 호출자 변수를 nullptr로 바꾼다.
	// 카운트가 남아 있어도 이 호출이 반환한 소유권은 다시 Release하면 안 되며 별도 소유권을 확보해야 한다.
	template<typename T>
	uint32_t Safe_Release(T& Instance)
	{
		uint32_t		iRefCnt = 0;

		if (nullptr != Instance)
		{
			iRefCnt = Instance->Release();

			if (0 == iRefCnt)
				Instance = nullptr;
		}
		return iRefCnt;
	}

	// 스마트 포인터의 소유 참조를 내려놓는다. 다른 shared_ptr 소유자가 있으면 대상 객체는 살아 있을 수 있다.
	template<typename T>
	void Safe_Reset(T& Instance)
	{
		if (nullptr != Instance)
		{
			Instance.reset();
			Instance = nullptr;
		}		
	}

}

#endif // Engine_Function_h__
