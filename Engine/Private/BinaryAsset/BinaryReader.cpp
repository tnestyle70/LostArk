// 차용 byte 커서와 파일 전체를 담는 출력 vector의 서로 다른 소유 경계를 구현한다.
#include "BinaryAsset/BinaryReader.h"

// 바이너리 파일 끝에서 크기를 얻고 시작으로 돌아와 출력 배열에 한 번 읽는다.
#include <fstream>
// ReadBytes/Skip의 명시적 범위 실패를 runtime_error로 전달한다.
#include <stdexcept>

// 각 포맷의 ReadMemory가 입력 구간을 검사한 뒤 사용한다. 두 포인터는 외부 저장소를 차용한다.
CBinaryReader::CBinaryReader(const void* pData, size_t size)
	: m_pCursor{static_cast<const uint8_t*>(pData)}, m_pEnd{static_cast<const uint8_t*>(pData) + size}
{
}

// 목적지와 남은 바이트를 먼저 검사한다. 이 명시적 검사 실패에서는 커서와 목적지를 변경하지 않는다.
void CBinaryReader::ReadBytes(void* pDestination, size_t byteCount)
{
	if (nullptr == pDestination || byteCount > Remaining())
		throw runtime_error("CBinaryReader read past EOF");

	memcpy(pDestination, m_pCursor, byteCount);
	m_pCursor += byteCount;
}

// 내용을 복사하지 않고 검증된 바이트 수만 건너뛴다. 실패하면 커서는 그대로다.
void CBinaryReader::Skip(size_t byteCount)
{
	if (byteCount > Remaining())
		throw runtime_error("CBinaryReader skip past EOF");

	m_pCursor += byteCount;
}

// 각 reader의 파일 진입점이다. 기존 출력을 보존하는 API가 아니며, 크기 할당 예외는 여기서 catch하지 않는다.
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

	// 일괄 read의 성공 여부를 반환한다. false일 때 부분 입력을 성공한 포맷으로 소비하지 않는 것은 호출자 책임이다.
	outBytes.resize(static_cast<size_t>(end));
	stream.seekg(0, ios::beg);
	return !!stream.read(reinterpret_cast<char_t*>(outBytes.data()), end);
}
