#pragma once

#include "Engine_Defines.h"

#include <filesystem>

NS_BEGIN(Engine)

class CBinaryReader final
{
public:
	CBinaryReader(const void* pData, size_t size);

	template<typename T>
	T Read()
	{
		static_assert(is_trivially_copyable_v<T>, "CBinaryReader only accepts POD values.");

		T value{};
		ReadBytes(&value, sizeof(T));
		return value;
	}

	void ReadBytes(void* pDestination, size_t byteCount);
	void Skip(size_t byteCount);

	const uint8_t* Peek() const { return m_pCursor; }
	size_t Remaining() const { return static_cast<size_t>(m_pEnd - m_pCursor); }

	static bool_t LoadFile(const filesystem::path& path, vector<uint8_t>& outBytes);

private:
	const uint8_t* m_pCursor = { nullptr };
	const uint8_t* m_pEnd = { nullptr };
};

NS_END
