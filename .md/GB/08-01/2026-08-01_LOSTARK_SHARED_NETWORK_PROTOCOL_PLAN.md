# LostArk Shared 네트워크 계약 수직 절편 구현 계획서 (검토용·미반영)

> 상태: 2026-08-01 사용자 요청으로 코드 반영을 전부 원복했다. 이 문서는 구현 승인이
> 아니라 원리와 자료구조를 함께 검토하기 위한 초안이다. 각 계약을 설명하고 동의하기
> 전에는 `Shared`, FlatBuffers 반입, 솔루션 등록을 다시 수행하지 않는다.

```text
문서 유형: 구현 계획서
출력 모드: CODE_WITH_EXPLANATION
C1~C8: OFF
문제 해결 ①~⑤: OFF
자료구조·알고리즘: ON
```

## 1. 파일별 최종 반영 코드

### 1.1 `C:/Users/user/Desktop/LostArk/Shared/Public/Identity/NetworkIds.h`

```cpp
#pragma once

#include <cstdint>

namespace LostArk::Shared
{
    template <typename Tag>
    class TNetworkId final
    {
    public:
        constexpr TNetworkId() noexcept = default;
        explicit constexpr TNetworkId(const std::uint32_t value) noexcept
            : m_value(value)
        {
        }

        [[nodiscard]] constexpr std::uint32_t Value() const noexcept
        {
            return m_value;
        }

        [[nodiscard]] constexpr bool IsValid() const noexcept
        {
            return 0u != m_value;
        }

        friend constexpr bool operator==(const TNetworkId&, const TNetworkId&) noexcept = default;

    private:
        std::uint32_t m_value = 0u;
    };

    struct PlayerIdTag final {};
    struct NetEntityIdTag final {};
    struct PartyIdTag final {};
    struct TransitionTokenTag final {};

    using PlayerId = TNetworkId<PlayerIdTag>;
    using NetEntityId = TNetworkId<NetEntityIdTag>;
    using PartyId = TNetworkId<PartyIdTag>;
    using TransitionToken = TNetworkId<TransitionTokenTag>;
}
```

### 1.2 `C:/Users/user/Desktop/LostArk/Shared/Public/Network/PacketType.h`

```cpp
#pragma once

#include <cstdint>

namespace LostArk::Shared
{
    enum class EPacketType : std::uint16_t
    {
        Session = 1u,
        ClientCommand = 2u,
        WorldSnapshot = 3u,
        WorldEvent = 4u,
        ZoneTransfer = 5u,
    };

    [[nodiscard]] constexpr bool IsKnownPacketType(const EPacketType type) noexcept
    {
        switch (type)
        {
        case EPacketType::Session:
        case EPacketType::ClientCommand:
        case EPacketType::WorldSnapshot:
        case EPacketType::WorldEvent:
        case EPacketType::ZoneTransfer:
            return true;
        default:
            return false;
        }
    }
}
```

### 1.3 `C:/Users/user/Desktop/LostArk/Shared/Public/Network/PacketEnvelope.h`

```cpp
#pragma once

#include "PacketType.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace LostArk::Shared
{
    inline constexpr std::uint32_t PACKET_MAGIC = 0x4B52414Cu;
    inline constexpr std::uint16_t PROTOCOL_VERSION = 1u;
    inline constexpr std::size_t PACKET_HEADER_SIZE = 16u;
    inline constexpr std::uint32_t MAX_PACKET_PAYLOAD_SIZE = 64u * 1024u;

    enum class EPacketFrameError : std::uint8_t
    {
        None,
        BufferLimitExceeded,
        InvalidMagic,
        UnsupportedVersion,
        UnknownPacketType,
        PayloadTooLarge,
    };

    struct PacketEnvelope final
    {
        std::uint32_t magic = PACKET_MAGIC;
        std::uint16_t version = PROTOCOL_VERSION;
        EPacketType packetType = EPacketType::Session;
        std::uint32_t payloadSize = 0u;
        std::uint32_t sequence = 0u;
    };

    using EncodedPacketEnvelope = std::array<std::uint8_t, PACKET_HEADER_SIZE>;

    [[nodiscard]] EncodedPacketEnvelope EncodePacketEnvelope(const PacketEnvelope& envelope) noexcept;
    [[nodiscard]] bool TryDecodePacketEnvelope(
        std::span<const std::uint8_t> bytes,
        PacketEnvelope& outEnvelope,
        EPacketFrameError& outError) noexcept;
    [[nodiscard]] bool TryBuildPacketFrame(
        EPacketType packetType,
        std::uint32_t sequence,
        std::span<const std::uint8_t> payload,
        std::vector<std::uint8_t>& outFrame,
        EPacketFrameError& outError);
}
```

### 1.4 `C:/Users/user/Desktop/LostArk/Shared/Private/Network/PacketEnvelope.cpp`

```cpp
#include "Network/PacketEnvelope.h"

#include <algorithm>

namespace
{
    void WriteUInt16LE(std::uint8_t* destination, const std::uint16_t value) noexcept
    {
        destination[0] = static_cast<std::uint8_t>(value & 0xFFu);
        destination[1] = static_cast<std::uint8_t>((value >> 8u) & 0xFFu);
    }

    void WriteUInt32LE(std::uint8_t* destination, const std::uint32_t value) noexcept
    {
        destination[0] = static_cast<std::uint8_t>(value & 0xFFu);
        destination[1] = static_cast<std::uint8_t>((value >> 8u) & 0xFFu);
        destination[2] = static_cast<std::uint8_t>((value >> 16u) & 0xFFu);
        destination[3] = static_cast<std::uint8_t>((value >> 24u) & 0xFFu);
    }

    [[nodiscard]] std::uint16_t ReadUInt16LE(const std::uint8_t* source) noexcept
    {
        return static_cast<std::uint16_t>(source[0]) |
            static_cast<std::uint16_t>(static_cast<std::uint16_t>(source[1]) << 8u);
    }

    [[nodiscard]] std::uint32_t ReadUInt32LE(const std::uint8_t* source) noexcept
    {
        return static_cast<std::uint32_t>(source[0]) |
            (static_cast<std::uint32_t>(source[1]) << 8u) |
            (static_cast<std::uint32_t>(source[2]) << 16u) |
            (static_cast<std::uint32_t>(source[3]) << 24u);
    }
}

namespace LostArk::Shared
{
    EncodedPacketEnvelope EncodePacketEnvelope(const PacketEnvelope& envelope) noexcept
    {
        EncodedPacketEnvelope bytes{};
        WriteUInt32LE(bytes.data(), envelope.magic);
        WriteUInt16LE(bytes.data() + 4u, envelope.version);
        WriteUInt16LE(bytes.data() + 6u, static_cast<std::uint16_t>(envelope.packetType));
        WriteUInt32LE(bytes.data() + 8u, envelope.payloadSize);
        WriteUInt32LE(bytes.data() + 12u, envelope.sequence);
        return bytes;
    }

    bool TryDecodePacketEnvelope(
        const std::span<const std::uint8_t> bytes,
        PacketEnvelope& outEnvelope,
        EPacketFrameError& outError) noexcept
    {
        outError = EPacketFrameError::None;
        if (bytes.size() < PACKET_HEADER_SIZE)
            return false;

        PacketEnvelope candidate{};
        candidate.magic = ReadUInt32LE(bytes.data());
        candidate.version = ReadUInt16LE(bytes.data() + 4u);
        candidate.packetType = static_cast<EPacketType>(ReadUInt16LE(bytes.data() + 6u));
        candidate.payloadSize = ReadUInt32LE(bytes.data() + 8u);
        candidate.sequence = ReadUInt32LE(bytes.data() + 12u);

        if (PACKET_MAGIC != candidate.magic)
            outError = EPacketFrameError::InvalidMagic;
        else if (PROTOCOL_VERSION != candidate.version)
            outError = EPacketFrameError::UnsupportedVersion;
        else if (!IsKnownPacketType(candidate.packetType))
            outError = EPacketFrameError::UnknownPacketType;
        else if (MAX_PACKET_PAYLOAD_SIZE < candidate.payloadSize)
            outError = EPacketFrameError::PayloadTooLarge;

        if (EPacketFrameError::None != outError)
            return false;

        outEnvelope = candidate;
        return true;
    }

    bool TryBuildPacketFrame(
        const EPacketType packetType,
        const std::uint32_t sequence,
        const std::span<const std::uint8_t> payload,
        std::vector<std::uint8_t>& outFrame,
        EPacketFrameError& outError)
    {
        outFrame.clear();
        outError = EPacketFrameError::None;

        if (!IsKnownPacketType(packetType))
        {
            outError = EPacketFrameError::UnknownPacketType;
            return false;
        }

        if (MAX_PACKET_PAYLOAD_SIZE < payload.size())
        {
            outError = EPacketFrameError::PayloadTooLarge;
            return false;
        }

        const PacketEnvelope envelope{
            PACKET_MAGIC,
            PROTOCOL_VERSION,
            packetType,
            static_cast<std::uint32_t>(payload.size()),
            sequence,
        };
        const EncodedPacketEnvelope header = EncodePacketEnvelope(envelope);

        outFrame.reserve(PACKET_HEADER_SIZE + payload.size());
        outFrame.insert(outFrame.end(), header.begin(), header.end());
        outFrame.insert(outFrame.end(), payload.begin(), payload.end());
        return true;
    }
}
```

### 1.5 `C:/Users/user/Desktop/LostArk/Shared/Public/Network/PacketFrameParser.h`

```cpp
#pragma once

#include "PacketEnvelope.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace LostArk::Shared
{
    inline constexpr std::size_t MAX_BUFFERED_PACKET_BYTES =
        (PACKET_HEADER_SIZE + MAX_PACKET_PAYLOAD_SIZE) * 4u;

    enum class EFrameParseStatus : std::uint8_t
    {
        NeedMoreData,
        FrameReady,
        InvalidFrame,
    };

    struct PacketFrame final
    {
        EPacketType packetType = EPacketType::Session;
        std::uint32_t sequence = 0u;
        std::vector<std::uint8_t> payload;
    };

    class CPacketFrameParser final
    {
    public:
        [[nodiscard]] bool Append(
            std::span<const std::uint8_t> bytes,
            EPacketFrameError& outError);
        [[nodiscard]] EFrameParseStatus TryPop(
            PacketFrame& outFrame,
            EPacketFrameError& outError);

        void Reset() noexcept;

        [[nodiscard]] std::size_t GetBufferedByteCount() const noexcept;

    private:
        void Compact();

    private:
        std::vector<std::uint8_t> m_buffer;
        std::size_t m_readOffset = 0u;
    };
}
```

### 1.6 `C:/Users/user/Desktop/LostArk/Shared/Private/Network/PacketFrameParser.cpp`

```cpp
#include "Network/PacketFrameParser.h"

#include <algorithm>

namespace LostArk::Shared
{
    bool CPacketFrameParser::Append(
        const std::span<const std::uint8_t> bytes,
        EPacketFrameError& outError)
    {
        outError = EPacketFrameError::None;
        Compact();

        if (MAX_BUFFERED_PACKET_BYTES - GetBufferedByteCount() < bytes.size())
        {
            outError = EPacketFrameError::BufferLimitExceeded;
            Reset();
            return false;
        }

        m_buffer.insert(m_buffer.end(), bytes.begin(), bytes.end());
        return true;
    }

    EFrameParseStatus CPacketFrameParser::TryPop(
        PacketFrame& outFrame,
        EPacketFrameError& outError)
    {
        outError = EPacketFrameError::None;
        const std::size_t bufferedByteCount = GetBufferedByteCount();
        if (bufferedByteCount < PACKET_HEADER_SIZE)
            return EFrameParseStatus::NeedMoreData;

        const std::span<const std::uint8_t> unreadBytes{
            m_buffer.data() + m_readOffset,
            bufferedByteCount,
        };
        PacketEnvelope envelope{};
        if (!TryDecodePacketEnvelope(unreadBytes.first(PACKET_HEADER_SIZE), envelope, outError))
        {
            Reset();
            return EFrameParseStatus::InvalidFrame;
        }

        const std::size_t frameByteCount = PACKET_HEADER_SIZE + envelope.payloadSize;
        if (bufferedByteCount < frameByteCount)
            return EFrameParseStatus::NeedMoreData;

        outFrame.packetType = envelope.packetType;
        outFrame.sequence = envelope.sequence;
        const std::uint8_t* payloadBegin = unreadBytes.data() + PACKET_HEADER_SIZE;
        outFrame.payload.assign(payloadBegin, payloadBegin + envelope.payloadSize);

        m_readOffset += frameByteCount;
        if (m_readOffset == m_buffer.size())
            Reset();

        return EFrameParseStatus::FrameReady;
    }

    void CPacketFrameParser::Reset() noexcept
    {
        m_buffer.clear();
        m_readOffset = 0u;
    }

    std::size_t CPacketFrameParser::GetBufferedByteCount() const noexcept
    {
        return m_buffer.size() - m_readOffset;
    }

    void CPacketFrameParser::Compact()
    {
        if (0u == m_readOffset)
            return;

        m_buffer.erase(m_buffer.begin(), m_buffer.begin() + static_cast<std::ptrdiff_t>(m_readOffset));
        m_readOffset = 0u;
    }
}
```

### 1.7 `C:/Users/user/Desktop/LostArk/Shared/Schemas/Session.fbs`

```text
namespace LostArk.Shared.Schema;

enum CharacterClass : ubyte {
  None = 0,
  Gunslinger = 1,
  Destroyer = 2,
  Artist = 3,
  Slayer = 4
}

table SessionHello {
  nickname:string (required);
  character:CharacterClass = None;
}

table SessionAccepted {
  player_id:uint;
  entity_id:uint;
}

union SessionPayload {
  SessionHello,
  SessionAccepted
}

table SessionPacket {
  payload:SessionPayload;
}

root_type SessionPacket;
file_identifier "LASE";
```

### 1.8 `C:/Users/user/Desktop/LostArk/Shared/Tools/GenerateSchemas.ps1`

```powershell
param(
    [Parameter(Mandatory = $false)]
    [string]$FlatcPath
)

$ErrorActionPreference = 'Stop'
$schemaRoot = Split-Path -Parent $PSScriptRoot
$generatedRoot = Join-Path $schemaRoot 'Schemas\Generated'

if ([string]::IsNullOrWhiteSpace($FlatcPath)) {
    $flatcCommand = Get-Command flatc -ErrorAction SilentlyContinue
    if ($null -eq $flatcCommand) {
        throw 'flatc를 찾지 못했습니다. -FlatcPath로 flatc.exe 경로를 지정하세요.'
    }
    $FlatcPath = $flatcCommand.Source
}

$resolvedFlatc = (Resolve-Path -LiteralPath $FlatcPath).Path
$sessionSchema = Join-Path $schemaRoot 'Schemas\Session.fbs'
New-Item -ItemType Directory -Force -Path $generatedRoot | Out-Null

& $resolvedFlatc --cpp --cpp-std c++17 --scoped-enums -o $generatedRoot $sessionSchema
if ($LASTEXITCODE -ne 0) {
    throw "flatc가 종료 코드 $LASTEXITCODE 를 반환했습니다."
}

Write-Host "Generated: $(Join-Path $generatedRoot 'Session_generated.h')"
```

### 1.9 `C:/Users/user/Desktop/LostArk/Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp`

```cpp
#include "Identity/NetworkIds.h"
#include "Network/PacketFrameParser.h"
#include "Network/PacketEnvelope.h"
#include "Session_generated.h"

#include <flatbuffers/flatbuffers.h>
#include <flatbuffers/verifier.h>

#include <array>
#include <cstdint>
#include <exception>
#include <iostream>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
    using namespace LostArk::Shared;

    void Require(const bool condition, const char* message)
    {
        if (!condition)
            throw std::runtime_error(message);
    }

    [[nodiscard]] std::vector<std::uint8_t> BuildSessionPayload()
    {
        flatbuffers::FlatBufferBuilder builder;
        const std::string nickname("\xEA\xB1\xB4\xEB\xB3\xB4");
        const auto nicknameOffset = builder.CreateString(nickname);
        const auto hello = LostArk::Shared::Schema::CreateSessionHello(
            builder,
            nicknameOffset,
            LostArk::Shared::Schema::CharacterClass::Gunslinger);
        const auto packet = LostArk::Shared::Schema::CreateSessionPacket(
            builder,
            LostArk::Shared::Schema::SessionPayload::SessionHello,
            hello.Union());
        LostArk::Shared::Schema::FinishSessionPacketBuffer(builder, packet);
        return {builder.GetBufferPointer(), builder.GetBufferPointer() + builder.GetSize()};
    }

    [[nodiscard]] std::vector<std::uint8_t> BuildFrame(
        const EPacketType packetType,
        const std::uint32_t sequence,
        const std::span<const std::uint8_t> payload)
    {
        std::vector<std::uint8_t> frame;
        EPacketFrameError error = EPacketFrameError::None;
        Require(TryBuildPacketFrame(packetType, sequence, payload, frame, error), "frame build failed");
        Require(EPacketFrameError::None == error, "frame build returned an error");
        return frame;
    }

    void TestStrongIds()
    {
        Require(!PlayerId{}.IsValid(), "default PlayerId must be invalid");
        Require(PlayerId{7u}.IsValid(), "non-zero PlayerId must be valid");
        Require(PlayerId{7u} == PlayerId{7u}, "same PlayerId values must compare equal");
        Require(!(PlayerId{7u} == PlayerId{8u}), "different PlayerId values must differ");
    }

    void TestSessionFlatBufferRoundTrip()
    {
        const std::vector<std::uint8_t> payload = BuildSessionPayload();
        flatbuffers::Verifier verifier(payload.data(), payload.size());
        Require(LostArk::Shared::Schema::VerifySessionPacketBuffer(verifier), "session payload must verify");

        const auto* packet = LostArk::Shared::Schema::GetSessionPacket(payload.data());
        Require(nullptr != packet, "session packet must exist");
        const auto* hello = packet->payload_as_SessionHello();
        Require(nullptr != hello, "session hello must exist");
        Require(hello->character() == LostArk::Shared::Schema::CharacterClass::Gunslinger,
            "character class must round-trip");
        Require(hello->nickname()->str() == std::string("\xEA\xB1\xB4\xEB\xB3\xB4"),
            "UTF-8 nickname must round-trip");
    }

    void TestByteByByteFrame()
    {
        const std::vector<std::uint8_t> payload = BuildSessionPayload();
        const std::vector<std::uint8_t> bytes = BuildFrame(EPacketType::Session, 41u, payload);
        CPacketFrameParser parser;
        PacketFrame frame;
        EPacketFrameError error = EPacketFrameError::None;

        for (std::size_t i = 0u; i < bytes.size(); ++i)
        {
            Require(parser.Append(std::span<const std::uint8_t>{bytes.data() + i, 1u}, error),
                "single byte append failed");
            const EFrameParseStatus status = parser.TryPop(frame, error);
            if (i + 1u < bytes.size())
                Require(EFrameParseStatus::NeedMoreData == status, "partial frame must wait");
            else
                Require(EFrameParseStatus::FrameReady == status, "complete frame must be ready");
        }

        Require(EPacketType::Session == frame.packetType, "packet type mismatch");
        Require(41u == frame.sequence, "sequence mismatch");
        Require(payload == frame.payload, "payload mismatch");
        Require(0u == parser.GetBufferedByteCount(), "parser must consume the frame");
    }

    void TestCoalescedFrames()
    {
        const std::array<std::uint8_t, 3u> firstPayload{1u, 2u, 3u};
        const std::array<std::uint8_t, 2u> secondPayload{9u, 8u};
        std::vector<std::uint8_t> bytes = BuildFrame(EPacketType::ClientCommand, 10u, firstPayload);
        const std::vector<std::uint8_t> second = BuildFrame(EPacketType::WorldEvent, 11u, secondPayload);
        bytes.insert(bytes.end(), second.begin(), second.end());

        CPacketFrameParser parser;
        PacketFrame frame;
        EPacketFrameError error = EPacketFrameError::None;
        Require(parser.Append(bytes, error), "coalesced append failed");
        Require(EFrameParseStatus::FrameReady == parser.TryPop(frame, error), "first frame missing");
        Require(10u == frame.sequence && EPacketType::ClientCommand == frame.packetType,
            "first frame mismatch");
        Require(EFrameParseStatus::FrameReady == parser.TryPop(frame, error), "second frame missing");
        Require(11u == frame.sequence && EPacketType::WorldEvent == frame.packetType,
            "second frame mismatch");
        Require(EFrameParseStatus::NeedMoreData == parser.TryPop(frame, error), "parser must be empty");
    }

    void TestInvalidHeadersResetParser()
    {
        const std::array<std::uint8_t, 1u> payload{1u};
        const std::vector<std::uint8_t> valid = BuildFrame(EPacketType::Session, 1u, payload);

        const auto runInvalidCase = [&valid](const std::size_t byteIndex, const std::uint8_t value,
            const EPacketFrameError expectedError)
        {
            std::vector<std::uint8_t> bytes = valid;
            bytes[byteIndex] = value;
            CPacketFrameParser parser;
            PacketFrame frame;
            EPacketFrameError error = EPacketFrameError::None;
            Require(parser.Append(bytes, error), "invalid case append failed");
            Require(EFrameParseStatus::InvalidFrame == parser.TryPop(frame, error),
                "invalid header must be rejected");
            Require(expectedError == error, "unexpected validation error");
            Require(0u == parser.GetBufferedByteCount(), "invalid frame must reset parser");
        };

        runInvalidCase(0u, 0u, EPacketFrameError::InvalidMagic);
        runInvalidCase(4u, 2u, EPacketFrameError::UnsupportedVersion);
        runInvalidCase(6u, 0xFFu, EPacketFrameError::UnknownPacketType);
    }

    void TestLimitsAndMalformedPayload()
    {
        std::vector<std::uint8_t> oversizedPayload(MAX_PACKET_PAYLOAD_SIZE + 1u, 0u);
        std::vector<std::uint8_t> frame;
        EPacketFrameError error = EPacketFrameError::None;
        Require(!TryBuildPacketFrame(EPacketType::Session, 1u, oversizedPayload, frame, error),
            "oversized payload must fail");
        Require(EPacketFrameError::PayloadTooLarge == error, "wrong oversized payload error");

        CPacketFrameParser parser;
        std::vector<std::uint8_t> oversizedBuffer(MAX_BUFFERED_PACKET_BYTES + 1u, 0u);
        Require(!parser.Append(oversizedBuffer, error), "oversized receive buffer must fail");
        Require(EPacketFrameError::BufferLimitExceeded == error, "wrong buffer limit error");
        Require(0u == parser.GetBufferedByteCount(), "buffer limit failure must reset parser");

        const std::array<std::uint8_t, 8u> malformedFlatBuffer{};
        flatbuffers::Verifier verifier(malformedFlatBuffer.data(), malformedFlatBuffer.size());
        Require(!LostArk::Shared::Schema::VerifySessionPacketBuffer(verifier),
            "malformed FlatBuffer must fail verification");
    }

    struct TestCase final
    {
        const char* name;
        void (*run)();
    };
}

int main()
{
    const std::array<TestCase, 6u> tests{
        TestCase{"StrongIds", &TestStrongIds},
        TestCase{"SessionFlatBufferRoundTrip", &TestSessionFlatBufferRoundTrip},
        TestCase{"ByteByByteFrame", &TestByteByByteFrame},
        TestCase{"CoalescedFrames", &TestCoalescedFrames},
        TestCase{"InvalidHeadersResetParser", &TestInvalidHeadersResetParser},
        TestCase{"LimitsAndMalformedPayload", &TestLimitsAndMalformedPayload},
    };

    std::size_t failureCount = 0u;
    for (const TestCase& test : tests)
    {
        try
        {
            test.run();
            std::cout << "[PASS] " << test.name << '\n';
        }
        catch (const std::exception& exception)
        {
            ++failureCount;
            std::cerr << "[FAIL] " << test.name << ": " << exception.what() << '\n';
        }
    }

    std::cout << "Tests: " << tests.size() << ", Failures: " << failureCount << '\n';
    return 0u == failureCount ? 0 : 1;
}
```

### 1.10 프로젝트 파일과 필터

`Shared/Default/Shared.vcxproj`는 `StaticLibrary`, `Tools/NetworkProtocolHarness/Default/NetworkProtocolHarness.vcxproj`는 `ConsoleApplication`으로 만든다. 두 프로젝트 모두 `Debug/Release × Win32/x64`, C++20, `/utf-8`을 사용한다.

정확한 등록 항목은 다음과 같다.

```xml
<!-- Shared.vcxproj -->
<ClInclude Include="..\Public\Identity\NetworkIds.h" />
<ClInclude Include="..\Public\Network\PacketType.h" />
<ClInclude Include="..\Public\Network\PacketEnvelope.h" />
<ClInclude Include="..\Public\Network\PacketFrameParser.h" />
<ClInclude Include="..\Schemas\Generated\Session_generated.h" />
<ClCompile Include="..\Private\Network\PacketEnvelope.cpp" />
<ClCompile Include="..\Private\Network\PacketFrameParser.cpp" />
<None Include="..\Schemas\Session.fbs" />
<None Include="..\Tools\GenerateSchemas.ps1" />

<!-- NetworkProtocolHarness.vcxproj -->
<ClCompile Include="..\Private\NetworkProtocolHarness.cpp" />
<ProjectReference Include="..\..\..\Shared\Default\Shared.vcxproj">
  <Project>{F4CCF815-6D51-412F-A76E-84D2F1D05571}</Project>
</ProjectReference>
```

필터는 `Public/Identity`, `Public/Network`, `Private/Network`, `Schemas`, `Generated`, `Tools`, `Private`만 추가한다. 물리 폴더와 동일한 방향으로 배치하며 기존 필터를 재배치하지 않는다.

### 1.11 생성·외부 파일

- `Shared/Schemas/Generated/Session_generated.h`: 1.7의 스키마를 FlatBuffers `flatc 25.12.19`로 생성한 파생 파일이다. 수동 편집하지 않고 1.8의 스크립트로 재생성한다.
- `ThirdParty/FlatBuffers/include/flatbuffers/*.h`: Google FlatBuffers 공식 `v25.12.19`의 header-only C++ 런타임을 그대로 가져온다.
- `ThirdParty/FlatBuffers/LICENSE.txt`: 동일 태그의 Apache-2.0 라이선스 원문을 그대로 가져온다.
- `flatc.exe`는 저장소에 넣지 않는다. 스키마 변경 담당자만 공식 배포본이나 명시 경로를 사용하고, 일반 빌드는 커밋된 생성 헤더를 소비한다.

## 2. 변경 범위

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 추가 | `C:/Users/user/Desktop/LostArk/Shared/Public/Identity/NetworkIds.h` | 서버가 부여하는 강타입 ID |
| 추가 | `C:/Users/user/Desktop/LostArk/Shared/Public/Network/PacketType.h` | 패킷 큰 범주 |
| 추가 | `C:/Users/user/Desktop/LostArk/Shared/Public/Network/PacketEnvelope.h` | 16바이트 TCP 프레임 헤더 계약 |
| 추가 | `C:/Users/user/Desktop/LostArk/Shared/Private/Network/PacketEnvelope.cpp` | 명시적 little-endian encode/decode |
| 추가 | `C:/Users/user/Desktop/LostArk/Shared/Public/Network/PacketFrameParser.h` | TCP 수신 바이트 누적/프레임 추출 계약 |
| 추가 | `C:/Users/user/Desktop/LostArk/Shared/Private/Network/PacketFrameParser.cpp` | 분할·합쳐진 TCP 수신 처리 |
| 추가 | `C:/Users/user/Desktop/LostArk/Shared/Schemas/Session.fbs` | 최초 접속 패킷 payload 스키마 |
| 추가 | `C:/Users/user/Desktop/LostArk/Shared/Schemas/Generated/Session_generated.h` | 생성된 FlatBuffers C++ 계약 |
| 추가 | `C:/Users/user/Desktop/LostArk/Shared/Tools/GenerateSchemas.ps1` | 스키마 재생성 명령 |
| 추가 | `C:/Users/user/Desktop/LostArk/Shared/Default/Shared.vcxproj` | Shared 정적 라이브러리 프로젝트 |
| 추가 | `C:/Users/user/Desktop/LostArk/Shared/Default/Shared.vcxproj.filters` | Shared 필터 |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/NetworkProtocolHarness/Private/NetworkProtocolHarness.cpp` | 헤드리스 회귀 검증 |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/NetworkProtocolHarness/Default/NetworkProtocolHarness.vcxproj` | 검증 실행기 프로젝트 |
| 추가 | `C:/Users/user/Desktop/LostArk/Tools/NetworkProtocolHarness/Default/NetworkProtocolHarness.vcxproj.filters` | 검증 실행기 필터 |
| 추가 | `C:/Users/user/Desktop/LostArk/ThirdParty/FlatBuffers` | 공식 FlatBuffers v25.12.19 헤더와 라이선스 |
| 수정 | `C:/Users/user/Desktop/LostArk/Framework.sln` | Shared와 검증 실행기 등록 |

### 배치와 의존성

| 파일/클래스 | 배치 | 소유 이유 | 직접 의존 | 의존 금지 | 수명/소유자 |
|---|---|---|---|---|---|
| `TNetworkId` | Shared | 서버·클라이언트 공통 ID 계약 | 표준 정수 | Engine, Client, Server 구현 | 값 수명 |
| `PacketEnvelope` | Shared | TCP 스트림 경계를 공통 해석 | `PacketType` | WinSock, IOCP | 프레임 값 수명 |
| `CPacketFrameParser` | Shared | 소켓 종류와 무관한 스트림 파싱 | `PacketEnvelope`, `vector` | WinSock, 게임 객체 | 연결별 parser 수명 |
| `Session.fbs` | Shared | 서버·클라이언트 공통 payload 정본 | FlatBuffers schema | Client UI, Server room | 저장소 정본 |
| 검증 실행기 | Tools | 렌더러 없이 계약을 반복 검증 | Shared, FlatBuffers | Engine, Client | 프로세스 수명 |

## 3. 코드 설명

한 문장 본질: TCP는 메시지 경계를 보존하지 않으므로, 서버와 클라이언트가 동일한 16바이트 헤더와 payload 스키마를 공유하고 임의로 분할·병합된 수신 바이트에서도 정확히 한 프레임씩 복원해야 한다.

전체 흐름은 다음과 같다.

```text
Session.fbs
  -> flatc가 Session_generated.h 생성
  -> FlatBufferBuilder가 SessionHello payload 생성
  -> TryBuildPacketFrame이 16바이트 little-endian 헤더를 앞에 붙임
  -> TCP/IOCP는 임의 크기의 byte chunk를 전달
  -> 연결별 CPacketFrameParser::Append
  -> TryPop이 header 검증 후 완전한 payload만 반환
  -> FlatBuffers Verifier 통과 후 SessionPacket을 읽음
```

`PacketEnvelope`는 네트워크 바이트를 C++ 구조체로 `memcpy`하지 않는다. 각 필드를 little-endian으로 직접 읽고 써서 컴파일러 padding과 ABI에 의존하지 않는다. `magic/version/type/size`가 틀리면 연결 단위 parser를 즉시 비우며 상위 네트워크 계층은 해당 연결을 종료할 수 있다.

`CPacketFrameParser`는 소켓을 모른다. IOCP 완료 통지에서 받은 바이트를 `Append`하고, `TryPop`이 `FrameReady`인 동안 반복 호출하면 된다. 따라서 향후 서버 워커 스레드 수와 클라이언트 수신 스레드 수가 달라도 프레이밍 계약은 변하지 않는다.

## 4. 자료구조·알고리즘 핵심

### 4.1 `TNetworkId<Tag>`

```text
표현하는 상태: 서버가 부여한 32비트 식별자. 0은 invalid.
선택 이유: PlayerId와 NetEntityId의 우연한 대입을 컴파일 단계에서 차단.
owner: 값의 원본은 서버, 복사본은 packet/runtime 소비자.
생성·파괴: 세션·엔티티 생성 시 발급, 값 객체 종료 시 파괴.
writer/reader: 서버 writer, 서버와 클라이언트 reader.
불변식: 0은 실체를 가리키지 않는다.
규모: ID 하나당 4바이트.
```

### 4.2 `CPacketFrameParser::m_buffer`

```text
표현하는 상태: 아직 완전한 프레임으로 소비되지 않은 TCP 수신 바이트.
선택 이유: TCP recv 한 번과 packet 한 개가 일치한다는 잘못된 가정을 제거.
owner: 연결 객체가 parser 하나를 소유.
writer: IOCP/클라이언트 수신 완료 처리.
reader: 해당 연결의 packet dispatch 단계.
불변식: unread byte 수는 MAX_BUFFERED_PACKET_BYTES 이하.
최대 규모: (16 + 65,536) * 4 = 262,208바이트/연결.
빈도: recv 완료마다 Append, tick/완료 처리에서 frame 수만큼 TryPop.
```

### 4.3 프레임 추출 알고리즘

```text
입력: 임의 길이 TCP byte chunk.
출력: NeedMoreData / FrameReady + PacketFrame / InvalidFrame + 오류.
처리: 누적 -> 16바이트 대기 -> header decode/validate -> payload 길이 대기 -> 한 frame 복사 -> read offset 이동.
종료 조건: 완전한 frame 하나를 반환하거나 현재 byte가 부족함.
실패 조건: magic/version/type/size/buffer 한도 위반.
실패 전파: 오류 enum 반환 및 parser reset. 상위 연결 owner가 disconnect 판단.
시간 복잡도: Append O(n), frame 반환 O(payload). Compact가 있을 때 남은 unread byte O(m).
공간 복잡도: 연결당 최대 262,208바이트 + 반환 payload 최대 65,536바이트.
호출 스레드: 연결의 수신 처리를 직렬화한 네트워크 스레드. 한 parser를 동시에 호출하지 않는다.
```

실제 값 흐름:

```text
nickname "건보" UTF-8 6바이트
-> SessionHello FlatBuffer payload
-> type=Session, sequence=41, payloadSize=N
-> [magic 4][version 2][type 2][size 4][sequence 4][payload N]
-> 1바이트씩 Append
-> 마지막 바이트 전까지 NeedMoreData
-> 마지막 바이트에서 FrameReady
-> Verifier 통과
-> nickname "건보", CharacterClass::Gunslinger 복원
```

## 5. 프로젝트 등록

- Shared 프로젝트 GUID: `{F4CCF815-6D51-412F-A76E-84D2F1D05571}`
- NetworkProtocolHarness 프로젝트 GUID: `{3BED234B-C5CE-4DDA-B154-4E4F947E6A50}`
- Harness는 `ProjectReference`로 Shared를 먼저 빌드한다.
- Framework.sln의 네 구성 `Debug/Release × x64/x86`에 두 프로젝트를 등록한다.
- 이번 단계는 Engine public header를 변경하지 않으므로 `UpdateLib.bat`은 회귀 빌드 단계에서만 실행한다.

## 6. 적용 순서와 검증

1. 공식 FlatBuffers `v25.12.19`의 `include/flatbuffers`와 `LICENSE.txt`를 `ThirdParty/FlatBuffers`에 가져온다.
2. `.fbs`, 생성 스크립트, Shared 코드를 추가한다.
3. Winters에서 실측한 `flatc 25.12.19`로 생성 스크립트를 한 번 실행하고 `Session_generated.h`를 저장한다.
4. Shared/Harness `.vcxproj`와 `.filters`를 추가하고 `Framework.sln`에 등록한다.
5. `Shared.vcxproj`와 `NetworkProtocolHarness.vcxproj`를 x64 Debug/Release로 빌드한다.
6. 두 구성의 `NetworkProtocolHarness.exe`를 실행하여 6개 테스트, 실패 0을 확인한다.
7. 저장소 공통 회귀 순서인 Engine x64 Debug/Release -> UpdateLib Debug/Release -> Client x64 Debug/Release를 실행한다.

성공 증거:

```text
[PASS] StrongIds
[PASS] SessionFlatBufferRoundTrip
[PASS] ByteByByteFrame
[PASS] CoalescedFrames
[PASS] InvalidHeadersResetParser
[PASS] LimitsAndMalformedPayload
Tests: 6, Failures: 0
```

실패 시 기존 상태 보존:

- 이 수직 절편은 새 디렉터리와 원래 깨끗했던 `Framework.sln`만 건드린다.
- 현재 다른 작업이 수정 중인 Engine/Object/Layer/GameInstance와 Client/Valtan 파일은 수정하지 않는다.
- 생성 실패 시 기존 `Session_generated.h`를 지우지 않도록 스크립트 개선이 필요하면 다음 단계에서 임시 출력 후 교체 방식으로 강화한다. 최초 생성인 이번 단계에는 기존 생성물이 없다.
