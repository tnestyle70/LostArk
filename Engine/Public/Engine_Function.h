#ifndef Engine_Function_h__
#define Engine_Function_h__

#include "Engine_Typedef.h"

namespace Engine
{
	// 템플릿은 기능의 정해져있으나 자료형은 정해져있지 않은 것
	// 기능을 인스턴스화 하기 위하여 만들어두는 틀

	template<typename T>
	void	Safe_Delete(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete Pointer;
			Pointer = nullptr;
		}
	}

	template<typename T>
	void	Safe_Delete_Array(T& Pointer)
	{
		if (nullptr != Pointer)
		{
			delete [] Pointer;
			Pointer = nullptr;
		}
	}

	template<typename T>
	uint32_t Safe_AddRef(T& Instance)
	{
		uint32_t		iRefCnt = 0;

		if (nullptr != Instance)
			iRefCnt = Instance->AddRef();

		return iRefCnt;
	}

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
