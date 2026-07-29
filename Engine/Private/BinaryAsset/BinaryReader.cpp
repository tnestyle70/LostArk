#include "BinaryAsset/BinaryReader.h"

#include <fstream>
#include <stdexcept>

CBinaryReader::CBinaryReader(const void* pData, size_t size)
	: m_pCursor { static_cast<const uint8_t*>(pData) }
	, m_pEnd { static_cast<const uint8_t*>(pData) + size }
{
}

void CBinaryReader::ReadBytes(void* pDestination, size_t byteCount)
{
	if (nullptr == pDestination || byteCount > Remaining())
		throw runtime_error("CBinaryReader read past EOF");

	memcpy(pDestination, m_pCursor, byteCount);
	m_pCursor += byteCount;
}

void CBinaryReader::Skip(size_t byteCount)
{
	if (byteCount > Remaining())
		throw runtime_error("CBinaryReader skip past EOF");

	m_pCursor += byteCount;
}

bool_t CBinaryReader::LoadFile(const filesystem::path& path, vector<uint8_t>& outBytes)
{
	outBytes.clear();

	ifstream stream(path, ios::binary | ios::ate);
	if (!stream)
		return false;

	const streamoff end = stream.tellg();
	if (end <= 0)
		return false;

	if (static_cast<uint64_t>(end) > static_cast<uint64_t>(SIZE_MAX))
		return false;

	outBytes.resize(static_cast<size_t>(end));
	stream.seekg(0, ios::beg);
	return !!stream.read(reinterpret_cast<char_t*>(outBytes.data()), end);
}
