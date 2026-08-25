#requires -Version 7.0

[CmdletBinding()]
param(
    [Parameter()]
    [string]$SourceRoot = 'C:\ProgramData\Smilegate\Games\LOSTARK\EFGame\ReleasePC\WwiseAudioPackage',

    [Parameter()]
    [string]$SelectionPath = '',

    [Parameter(Mandatory = $true)]
    [string]$WorkRoot,

    [Parameter()]
    [string]$KeyPath = '',

    [Parameter()]
    [long]$KeySampleBytesPerPck = 8MB,

    [Parameter()]
    [switch]$SkipStandaloneMedia
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$ExpectedKeyLength = 0x356
$ExpectedKeyPrefix = 'A6905F3A'
$ExpectedKeySha256 = '3B50D57DF7173E43F616CDAF4CEAEFC1C504FA83523ED189D463D68E145D0561'

$AudioCodecSource = @'
using System;
using System.Buffers.Binary;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;

public sealed class LostArkAkpkEntry
{
    public string Kind { get; init; } = string.Empty;
    public ulong Id { get; init; }
    public uint BlockSize { get; init; }
    public ulong Size { get; init; }
    public ulong Offset { get; init; }
    public uint LanguageId { get; init; }
}

public sealed class LostArkAkpkDocument
{
    public uint HeaderSize { get; init; }
    public long PlainPayloadSize { get; init; }
    public List<LostArkAkpkEntry> Entries { get; init; } = new();
}

public sealed class LostArkBnkMediaEntry
{
    public uint MediaId { get; init; }
    public uint Offset { get; init; }
    public uint Size { get; init; }
}

public static class LostArkAudioCodec
{
    private const int KeyPeriod = 0x356;
    private static readonly byte[] WrapperMagic = { 0x3e, 0xce, 0xa6, 0x74 };

    public static string DeobfuscateName(string source)
    {
        if (string.IsNullOrWhiteSpace(source))
            return source;

        var normalized = new string(source.Where(c => !char.IsWhiteSpace(c)).ToArray()).ToUpperInvariant();
        var mapped = new StringBuilder(normalized.Length);
        foreach (var c in normalized)
        {
            var x = (int)c;
            if (c >= '0' && c <= '9')
                x += 43;

            var value = (31 * (x - normalized.Length - 65) % 36 + 36) % 36 + 65;
            if (value >= 91)
                value -= 43;
            mapped.Append((char)value);
        }

        var substitutions = new (string Key, char Value, int PositionMod4)[]
        {
            ("QP", 'Q', 0), ("QD", 'Q', 1), ("QW", 'Q', 2), ("Q4", 'Q', 3),
            ("QL", '-', 0), ("QB", '-', 1), ("QO", '-', 2), ("Q5", '-', 3),
            ("QC", '_', 0), ("QN", '_', 1), ("QT", '_', 2), ("Q9", '_', 3),
            ("XU", 'X', 0), ("XN", 'X', 1), ("XH", 'X', 2), ("X3", 'X', 3),
            ("XW", '!', 0), ("XS", '!', 1), ("XZ", '!', 2), ("X0", '!', 3),
        };

        var raw = mapped.ToString();
        var cleaned = new StringBuilder(raw.Length);
        for (var index = 0; index < raw.Length;)
        {
            var matched = false;
            foreach (var substitution in substitutions)
            {
                if (index % 4 != substitution.PositionMod4 ||
                    !raw.AsSpan(index).StartsWith(substitution.Key, StringComparison.Ordinal))
                    continue;

                cleaned.Append(substitution.Value);
                index += substitution.Key.Length;
                matched = true;
                break;
            }

            if (!matched)
            {
                cleaned.Append(raw[index]);
                index++;
            }
        }

        var result = cleaned.ToString();
        var bang = result.IndexOf('!');
        return bang >= 0 ? result[..bang] : result;
    }

    public static byte[] RecoverKey(string sourceRoot, long maxBytesPerPck)
    {
        if (maxBytesPerPck <= 0)
            throw new ArgumentOutOfRangeException(nameof(maxBytesPerPck));

        var counts = new int[KeyPeriod, 256];
        var paths = Directory.EnumerateFiles(sourceRoot, "*.pck", SearchOption.AllDirectories).ToArray();
        if (paths.Length == 0)
            throw new InvalidDataException("No PCK files were found for key recovery.");

        var buffer = new byte[1 << 20];
        foreach (var path in paths)
        {
            using var input = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read, buffer.Length, FileOptions.SequentialScan);
            if (input.Length <= WrapperMagic.Length)
                continue;

            ValidateWrapperMagic(input, path);
            input.Position = WrapperMagic.Length;
            long remaining = Math.Min(maxBytesPerPck, input.Length - WrapperMagic.Length);
            long payloadOffset = 0;
            while (remaining > 0)
            {
                var read = input.Read(buffer, 0, (int)Math.Min(buffer.Length, remaining));
                if (read <= 0)
                    throw new EndOfStreamException($"Unexpected EOF while sampling {path}.");

                for (var index = 0; index < read; index++)
                {
                    var absolute = payloadOffset + index;
                    var keyIndex = (int)(absolute % KeyPeriod);
                    var blockMask = (byte)((absolute / KeyPeriod) % 0xff);
                    counts[keyIndex, buffer[index] ^ blockMask]++;
                }

                payloadOffset += read;
                remaining -= read;
            }
        }

        var key = new byte[KeyPeriod];
        for (var keyIndex = 0; keyIndex < KeyPeriod; keyIndex++)
        {
            var bestCount = -1;
            var bestValue = 0;
            for (var value = 0; value < 256; value++)
            {
                if (counts[keyIndex, value] <= bestCount)
                    continue;
                bestCount = counts[keyIndex, value];
                bestValue = value;
            }
            key[keyIndex] = (byte)bestValue;
        }

        return key;
    }

    public static LostArkAkpkDocument ReadAkpk(string path, byte[] key)
    {
        ValidateKey(key);
        using var input = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read);
        ValidateWrapperMagic(input, path);
        var plainPayloadSize = input.Length - WrapperMagic.Length;
        if (plainPayloadSize < 32)
            throw new InvalidDataException($"PCK payload is too small: {path}");

        var prefix = ReadDecryptedRange(input, key, 0, 32);
        if (!prefix.AsSpan(0, 4).SequenceEqual("AKPK"u8))
            throw new InvalidDataException($"Decrypted PCK does not start with AKPK: {path}");

        var headerSize = ReadUInt32(prefix, 4);
        var headerBytes = checked((long)headerSize + 8L);
        if (headerBytes > plainPayloadSize || headerBytes > 64L * 1024L * 1024L)
            throw new InvalidDataException($"Invalid AKPK header size {headerSize}: {path}");

        var header = ReadDecryptedRange(input, key, 0, checked((int)headerBytes));
        var section1Size = ReadUInt32(header, 12);
        var section2Size = ReadUInt32(header, 16);
        var section3Size = ReadUInt32(header, 20);
        var section4Size = 0u;
        var sectionOffset = 24;
        if ((long)section1Size + section2Size + section3Size + 0x10L < headerSize)
        {
            section4Size = ReadUInt32(header, 24);
            sectionOffset = 28;
        }

        var expectedEnd = checked((long)sectionOffset + section1Size + section2Size + section3Size + section4Size);
        if (expectedEnd > header.Length)
            throw new InvalidDataException($"AKPK sections exceed header bounds: {path}");

        var entries = new List<LostArkAkpkEntry>();
        ParseTable(header, checked(sectionOffset + (int)section1Size), section2Size, "bnk", entries, plainPayloadSize);
        ParseTable(header, checked(sectionOffset + (int)section1Size + (int)section2Size), section3Size, "wem", entries, plainPayloadSize);
        ParseTable(header, checked(sectionOffset + (int)section1Size + (int)section2Size + (int)section3Size), section4Size, "external", entries, plainPayloadSize);

        var duplicates = entries
            .Where(entry => entry.Kind != "external")
            .GroupBy(entry => (entry.Kind, entry.Id, entry.LanguageId))
            .Where(group => group.Count() > 1)
            .Select(group => $"{group.Key.Kind}:{group.Key.Id}:{group.Key.LanguageId}")
            .ToArray();
        if (duplicates.Length != 0)
            throw new InvalidDataException($"Duplicate AKPK entries: {string.Join(", ", duplicates)}");

        return new LostArkAkpkDocument
        {
            HeaderSize = headerSize,
            PlainPayloadSize = plainPayloadSize,
            Entries = entries,
        };
    }

    public static void ExtractWrappedRange(string inputPath, string outputPath, byte[] key, ulong plainOffset, ulong size)
    {
        ValidateKey(key);
        using var input = new FileStream(inputPath, FileMode.Open, FileAccess.Read, FileShare.Read, 1 << 20, FileOptions.SequentialScan);
        ValidateWrapperMagic(input, inputPath);
        var payloadSize = checked((ulong)(input.Length - WrapperMagic.Length));
        if (plainOffset > payloadSize || size > payloadSize - plainOffset)
            throw new InvalidDataException($"Requested wrapper range exceeds payload bounds: {inputPath}");

        input.Position = checked(WrapperMagic.Length + (long)plainOffset);
        using var output = new FileStream(outputPath, FileMode.CreateNew, FileAccess.Write, FileShare.None, 1 << 20, FileOptions.SequentialScan);
        var buffer = new byte[1 << 20];
        var remaining = size;
        var payloadOffset = plainOffset;
        while (remaining > 0)
        {
            var read = input.Read(buffer, 0, (int)Math.Min((ulong)buffer.Length, remaining));
            if (read <= 0)
                throw new EndOfStreamException($"Unexpected EOF while decrypting {inputPath}.");

            for (var index = 0; index < read; index++)
            {
                var absolute = payloadOffset + (ulong)index;
                var mask = (byte)(key[absolute % KeyPeriod] ^ ((absolute / KeyPeriod) % 0xff));
                buffer[index] ^= mask;
            }

            output.Write(buffer, 0, read);
            payloadOffset += (uint)read;
            remaining -= (uint)read;
        }
    }

    public static void DecryptWholeWrapper(string inputPath, string outputPath, byte[] key)
    {
        var length = new FileInfo(inputPath).Length;
        if (length <= WrapperMagic.Length)
            throw new InvalidDataException($"Wrapped file is too small: {inputPath}");
        ExtractWrappedRange(inputPath, outputPath, key, 0, checked((ulong)(length - WrapperMagic.Length)));
    }

    public static List<LostArkBnkMediaEntry> ReadBnkMedia(string path)
    {
        var bytes = File.ReadAllBytes(path);
        if (bytes.Length < 8 || !bytes.AsSpan(0, 4).SequenceEqual("BKHD"u8))
            throw new InvalidDataException($"BNK does not start with BKHD: {path}");

        var chunks = new Dictionary<string, (int Offset, int Size)>(StringComparer.Ordinal);
        for (var offset = 0; offset < bytes.Length;)
        {
            if (offset + 8 > bytes.Length)
                throw new InvalidDataException($"Truncated BNK chunk header: {path}");
            var id = Encoding.ASCII.GetString(bytes, offset, 4);
            var size = checked((int)ReadUInt32(bytes, offset + 4));
            var dataOffset = checked(offset + 8);
            var endOffset = checked(dataOffset + size);
            if (endOffset > bytes.Length)
                throw new InvalidDataException($"BNK chunk {id} exceeds file bounds: {path}");
            chunks[id] = (dataOffset, size);
            offset = endOffset;
        }

        if (!chunks.TryGetValue("DIDX", out var didx))
            return new List<LostArkBnkMediaEntry>();
        if (!chunks.TryGetValue("DATA", out var data))
            throw new InvalidDataException($"BNK has DIDX without DATA: {path}");
        if (didx.Size % 12 != 0)
            throw new InvalidDataException($"BNK DIDX size is not divisible by 12: {path}");

        var entries = new List<LostArkBnkMediaEntry>();
        for (var cursor = didx.Offset; cursor < didx.Offset + didx.Size; cursor += 12)
        {
            var id = ReadUInt32(bytes, cursor);
            var relativeOffset = ReadUInt32(bytes, cursor + 4);
            var size = ReadUInt32(bytes, cursor + 8);
            if ((ulong)relativeOffset + size > (ulong)data.Size)
                throw new InvalidDataException($"BNK media {id} exceeds DATA bounds: {path}");
            entries.Add(new LostArkBnkMediaEntry { MediaId = id, Offset = checked((uint)(data.Offset + relativeOffset)), Size = size });
        }

        if (entries.GroupBy(entry => entry.MediaId).Any(group => group.Count() > 1))
            throw new InvalidDataException($"BNK contains duplicate media IDs: {path}");
        return entries;
    }

    public static void ExtractPlainRange(string inputPath, string outputPath, ulong offset, ulong size)
    {
        using var input = new FileStream(inputPath, FileMode.Open, FileAccess.Read, FileShare.Read, 1 << 20, FileOptions.SequentialScan);
        if (offset > (ulong)input.Length || size > (ulong)input.Length - offset)
            throw new InvalidDataException($"Plain range exceeds file bounds: {inputPath}");
        input.Position = checked((long)offset);
        using var output = new FileStream(outputPath, FileMode.CreateNew, FileAccess.Write, FileShare.None, 1 << 20, FileOptions.SequentialScan);
        var buffer = new byte[1 << 20];
        var remaining = size;
        while (remaining > 0)
        {
            var read = input.Read(buffer, 0, (int)Math.Min((ulong)buffer.Length, remaining));
            if (read <= 0)
                throw new EndOfStreamException($"Unexpected EOF while extracting {inputPath}.");
            output.Write(buffer, 0, read);
            remaining -= (uint)read;
        }
    }

    public static string ReadMagic(string path)
    {
        using var input = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read);
        var bytes = new byte[4];
        if (input.Read(bytes, 0, bytes.Length) != bytes.Length)
            return string.Empty;
        return Encoding.ASCII.GetString(bytes);
    }

    public static string Sha256(byte[] bytes) => Convert.ToHexString(SHA256.HashData(bytes));

    public static string Sha256File(string path)
    {
        using var input = File.OpenRead(path);
        return Convert.ToHexString(SHA256.HashData(input));
    }

    private static void ParseTable(
        byte[] header,
        int offset,
        uint sectionSize,
        string kind,
        List<LostArkAkpkEntry> output,
        long payloadSize)
    {
        if (sectionSize == 0)
            return;
        if (sectionSize < 4 || offset < 0 || (long)offset + sectionSize > header.Length)
            throw new InvalidDataException($"Invalid AKPK {kind} table bounds.");

        var count = ReadUInt32(header, offset);
        if (count == 0)
        {
            if (sectionSize != 4)
                throw new InvalidDataException($"Empty AKPK {kind} table has unexpected size {sectionSize}.");
            return;
        }

        var payloadBytes = checked((int)sectionSize - 4);
        if (payloadBytes % count != 0)
            throw new InvalidDataException($"AKPK {kind} table size is not divisible by entry count.");
        var entrySize = payloadBytes / checked((int)count);
        if (entrySize != 20 && entrySize != 24)
            throw new InvalidDataException($"Unsupported AKPK {kind} entry size {entrySize}.");

        var cursor = offset + 4;
        for (var index = 0; index < count; index++)
        {
            ulong id;
            var fieldOffset = 0;
            if (entrySize == 24 && kind == "external")
            {
                id = BinaryPrimitives.ReadUInt64LittleEndian(header.AsSpan(cursor, 8));
                fieldOffset = 8;
            }
            else
            {
                id = ReadUInt32(header, cursor);
                fieldOffset = 4;
            }

            var blockSize = ReadUInt32(header, cursor + fieldOffset);
            fieldOffset += 4;
            ulong size;
            if (entrySize == 24 && kind != "external")
            {
                size = BinaryPrimitives.ReadUInt64LittleEndian(header.AsSpan(cursor + fieldOffset, 8));
                fieldOffset += 8;
            }
            else
            {
                size = ReadUInt32(header, cursor + fieldOffset);
                fieldOffset += 4;
            }

            var startBlock = ReadUInt32(header, cursor + fieldOffset);
            fieldOffset += 4;
            var languageId = ReadUInt32(header, cursor + fieldOffset);
            var plainOffset = blockSize == 0 ? startBlock : checked((ulong)startBlock * blockSize);
            if (kind != "external" && (plainOffset > (ulong)payloadSize || size > (ulong)payloadSize - plainOffset))
                throw new InvalidDataException($"AKPK {kind} entry {id} exceeds payload bounds.");

            output.Add(new LostArkAkpkEntry
            {
                Kind = kind,
                Id = id,
                BlockSize = blockSize,
                Size = size,
                Offset = plainOffset,
                LanguageId = languageId,
            });
            cursor += entrySize;
        }
    }

    private static byte[] ReadDecryptedRange(FileStream input, byte[] key, ulong offset, int size)
    {
        if (offset > (ulong)(input.Length - WrapperMagic.Length) || (ulong)size > (ulong)(input.Length - WrapperMagic.Length) - offset)
            throw new InvalidDataException("Requested decrypted range exceeds wrapper bounds.");
        input.Position = checked(WrapperMagic.Length + (long)offset);
        var bytes = new byte[size];
        input.ReadExactly(bytes);
        for (var index = 0; index < bytes.Length; index++)
        {
            var absolute = offset + (ulong)index;
            bytes[index] ^= (byte)(key[absolute % KeyPeriod] ^ ((absolute / KeyPeriod) % 0xff));
        }
        return bytes;
    }

    private static void ValidateWrapperMagic(FileStream input, string path)
    {
        input.Position = 0;
        Span<byte> magic = stackalloc byte[WrapperMagic.Length];
        if (input.Read(magic) != magic.Length || !magic.SequenceEqual(WrapperMagic))
            throw new InvalidDataException($"Unexpected Lost Ark audio wrapper magic: {path}");
    }

    private static void ValidateKey(byte[] key)
    {
        if (key == null || key.Length != KeyPeriod)
            throw new InvalidDataException($"Expected a {KeyPeriod}-byte Lost Ark audio key.");
    }

    private static uint ReadUInt32(byte[] bytes, int offset)
    {
        if (offset < 0 || offset + 4 > bytes.Length)
            throw new InvalidDataException("Unexpected end of binary data.");
        return BinaryPrimitives.ReadUInt32LittleEndian(bytes.AsSpan(offset, 4));
    }
}
'@

Add-Type -TypeDefinition $AudioCodecSource -Language CSharp

function Resolve-AbsolutePath {
    param([Parameter(Mandatory = $true)][string]$Path)
    return [System.IO.Path]::GetFullPath($Path)
}

function Assert-EmptyOrMissingDirectory {
    param([Parameter(Mandatory = $true)][string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        return
    }
    if (-not (Test-Path -LiteralPath $Path -PathType Container)) {
        throw "WorkRoot exists and is not a directory: $Path"
    }
    if (Get-ChildItem -LiteralPath $Path -Force | Select-Object -First 1) {
        throw "WorkRoot must be missing or empty so an existing stage is never overwritten: $Path"
    }
}

function Assert-Key {
    param([Parameter(Mandatory = $true)][byte[]]$Key)

    if ($Key.Length -ne $ExpectedKeyLength) {
        throw "Expected a $ExpectedKeyLength-byte key, got $($Key.Length)."
    }
    $prefix = [Convert]::ToHexString($Key[0..3])
    if ($prefix -ne $ExpectedKeyPrefix) {
        throw "Recovered key prefix mismatch: $prefix"
    }
    $sha = [LostArkAudioCodec]::Sha256($Key)
    if ($sha -ne $ExpectedKeySha256) {
        throw "Recovered key SHA-256 mismatch: $sha"
    }
}

function Get-UniqueLogicalIndex {
    param(
        [Parameter(Mandatory = $true)][System.IO.FileInfo[]]$Files,
        [Parameter(Mandatory = $true)][string]$Kind
    )

    $index = @{}
    foreach ($file in $Files) {
        $logical = [LostArkAudioCodec]::DeobfuscateName($file.BaseName)
        if ($index.ContainsKey($logical)) {
            throw "Duplicate logical $Kind identity '$logical': '$($index[$logical].FullName)' and '$($file.FullName)'"
        }
        $index[$logical] = $file
    }
    return $index
}

function Add-StagedMedia {
    param(
        [Parameter(Mandatory = $true)][string]$CandidatePath,
        [Parameter(Mandatory = $true)][uint64]$MediaId,
        [Parameter(Mandatory = $true)][string]$Origin,
        [Parameter(Mandatory = $true)][string]$MediaDirectory,
        [Parameter(Mandatory = $true)][hashtable]$MediaOrigins
    )

    if ([LostArkAudioCodec]::ReadMagic($CandidatePath) -ne 'RIFF') {
        throw "Decoded media $MediaId from '$Origin' is not a RIFF WEM."
    }

    $target = Join-Path $MediaDirectory ("{0}.wem" -f $MediaId)
    if (Test-Path -LiteralPath $target) {
        $candidateHash = [LostArkAudioCodec]::Sha256File($CandidatePath)
        $targetHash = [LostArkAudioCodec]::Sha256File($target)
        if ($candidateHash -ne $targetHash) {
            throw "Media ID $MediaId resolves to different payloads: '$target' and '$CandidatePath'"
        }
        Remove-Item -LiteralPath $CandidatePath
    }
    else {
        Move-Item -LiteralPath $CandidatePath -Destination $target
    }

    if (-not $MediaOrigins.ContainsKey([string]$MediaId)) {
        $MediaOrigins[[string]$MediaId] = [System.Collections.Generic.List[string]]::new()
    }
    if (-not $MediaOrigins[[string]$MediaId].Contains($Origin)) {
        $MediaOrigins[[string]$MediaId].Add($Origin)
    }
}

$SourceRoot = Resolve-AbsolutePath $SourceRoot
if ([string]::IsNullOrWhiteSpace($SelectionPath)) {
    $SelectionPath = Join-Path $PSScriptRoot 'LostArkSoundSelection.json'
}
$SelectionPath = Resolve-AbsolutePath $SelectionPath
$WorkRoot = Resolve-AbsolutePath $WorkRoot
if (-not (Test-Path -LiteralPath $SourceRoot -PathType Container)) {
    throw "Lost Ark Wwise source root does not exist: $SourceRoot"
}
if (-not (Test-Path -LiteralPath $SelectionPath -PathType Leaf)) {
    throw "Sound selection document does not exist: $SelectionPath"
}
Assert-EmptyOrMissingDirectory $WorkRoot
New-Item -ItemType Directory -Path $WorkRoot -Force | Out-Null

$BankRoot = Join-Path $WorkRoot 'banks'
$MediaRoot = Join-Path $WorkRoot 'media'
$ScratchRoot = Join-Path $WorkRoot 'scratch'
New-Item -ItemType Directory -Path $BankRoot, $MediaRoot, $ScratchRoot -Force | Out-Null

$selection = Get-Content -LiteralPath $SelectionPath -Raw | ConvertFrom-Json
if ($selection.formatVersion -ne 1) {
    throw "Unsupported sound selection formatVersion '$($selection.formatVersion)'."
}
$selectedNames = @($selection.packages | ForEach-Object { [string]$_.logicalName })
if ($selectedNames.Count -ne ($selectedNames | Sort-Object -Unique).Count) {
    throw 'Sound selection contains duplicate logical package names.'
}

$pckFiles = @(Get-ChildItem -LiteralPath $SourceRoot -Recurse -File -Filter '*.pck')
$wemFiles = @(Get-ChildItem -LiteralPath $SourceRoot -Recurse -File -Filter '*.wem')
$pckIndex = Get-UniqueLogicalIndex -Files $pckFiles -Kind 'PCK'
$wemIndex = Get-UniqueLogicalIndex -Files $wemFiles -Kind 'WEM'

foreach ($logicalName in $selectedNames) {
    if (-not $pckIndex.ContainsKey($logicalName)) {
        throw "Selected logical PCK '$logicalName' was not found under '$SourceRoot'."
    }
}

if ([string]::IsNullOrWhiteSpace($KeyPath)) {
    Write-Host "Recovering the Korean Wwise key from $($pckFiles.Count) PCK files..."
    $key = [LostArkAudioCodec]::RecoverKey($SourceRoot, $KeySampleBytesPerPck)
    Assert-Key $key
    $KeyPath = Join-Path $WorkRoot 'kr-audio.key'
    [System.IO.File]::WriteAllBytes($KeyPath, $key)
}
else {
    $KeyPath = Resolve-AbsolutePath $KeyPath
    if (-not (Test-Path -LiteralPath $KeyPath -PathType Leaf)) {
        throw "Key file does not exist: $KeyPath"
    }
    $key = [System.IO.File]::ReadAllBytes($KeyPath)
    Assert-Key $key
}

$packageResults = [System.Collections.Generic.List[object]]::new()
$mediaOrigins = @{}

$packageNumber = 0
foreach ($packageSelection in $selection.packages) {
    $packageNumber++
    $logicalName = [string]$packageSelection.logicalName
    $sourceFile = $pckIndex[$logicalName]
    Write-Host ("[{0}/{1}] Staging {2}" -f $packageNumber, $selectedNames.Count, $logicalName)

    $document = [LostArkAudioCodec]::ReadAkpk($sourceFile.FullName, $key)
    $packageBankRoot = Join-Path $BankRoot $logicalName
    New-Item -ItemType Directory -Path $packageBankRoot -Force | Out-Null
    $bankIds = [System.Collections.Generic.List[uint64]]::new()
    $packageMediaIds = [System.Collections.Generic.HashSet[uint64]]::new()

    foreach ($entry in $document.Entries) {
        if ($entry.Kind -eq 'external') {
            continue
        }

        if ($entry.Kind -eq 'bnk') {
            $bankPath = Join-Path $packageBankRoot ("{0}.bnk" -f $entry.Id)
            [LostArkAudioCodec]::ExtractWrappedRange($sourceFile.FullName, $bankPath, $key, $entry.Offset, $entry.Size)
            if ([LostArkAudioCodec]::ReadMagic($bankPath) -ne 'BKHD') {
                throw "Extracted bank $($entry.Id) from '$logicalName' does not start with BKHD."
            }
            $bankIds.Add($entry.Id)

            foreach ($mediaEntry in [LostArkAudioCodec]::ReadBnkMedia($bankPath)) {
                $candidate = Join-Path $ScratchRoot ("{0}-{1}-{2}.wem" -f $logicalName, $entry.Id, $mediaEntry.MediaId)
                [LostArkAudioCodec]::ExtractPlainRange($bankPath, $candidate, $mediaEntry.Offset, $mediaEntry.Size)
                Add-StagedMedia -CandidatePath $candidate -MediaId $mediaEntry.MediaId `
                    -Origin ("bnk:{0}:{1}" -f $logicalName, $entry.Id) -MediaDirectory $MediaRoot -MediaOrigins $mediaOrigins
                $null = $packageMediaIds.Add($mediaEntry.MediaId)
            }
            continue
        }

        if ($entry.Kind -eq 'wem') {
            $candidate = Join-Path $ScratchRoot ("{0}-{1}.wem" -f $logicalName, $entry.Id)
            [LostArkAudioCodec]::ExtractWrappedRange($sourceFile.FullName, $candidate, $key, $entry.Offset, $entry.Size)
            Add-StagedMedia -CandidatePath $candidate -MediaId $entry.Id `
                -Origin ("pck:{0}" -f $logicalName) -MediaDirectory $MediaRoot -MediaOrigins $mediaOrigins
            $null = $packageMediaIds.Add($entry.Id)
        }
    }

    $packageResults.Add([ordered]@{
        logicalName = $logicalName
        categories = @($packageSelection.categories)
        physicalFile = $sourceFile.Name
        sourceBytes = $sourceFile.Length
        headerSize = $document.HeaderSize
        bankIds = @($bankIds | Sort-Object)
        embeddedMediaIds = @($packageMediaIds | Sort-Object)
    })
}

if (-not $SkipStandaloneMedia) {
    $standaloneNumber = 0
    foreach ($pair in $wemIndex.GetEnumerator() | Sort-Object Key) {
        $standaloneNumber++
        if ($pair.Key -notmatch '^\d+$') {
            throw "Standalone WEM logical name is not a numeric media ID: '$($pair.Key)'"
        }
        $mediaId = [uint64]::Parse($pair.Key, [Globalization.CultureInfo]::InvariantCulture)
        $candidate = Join-Path $ScratchRoot ("standalone-{0}.wem" -f $mediaId)
        [LostArkAudioCodec]::DecryptWholeWrapper($pair.Value.FullName, $candidate, $key)
        Add-StagedMedia -CandidatePath $candidate -MediaId $mediaId -Origin 'standalone' `
            -MediaDirectory $MediaRoot -MediaOrigins $mediaOrigins

        if ($standaloneNumber % 100 -eq 0 -or $standaloneNumber -eq $wemIndex.Count) {
            Write-Host ("Staged standalone WEM {0}/{1}" -f $standaloneNumber, $wemIndex.Count)
        }
    }
}

$mediaResults = [System.Collections.Generic.List[object]]::new()
foreach ($mediaFile in Get-ChildItem -LiteralPath $MediaRoot -File -Filter '*.wem' | Sort-Object Name) {
    $mediaId = $mediaFile.BaseName
    $mediaResults.Add([ordered]@{
        mediaId = [uint64]$mediaId
        stagedFile = "media/$($mediaFile.Name)"
        bytes = $mediaFile.Length
        sha256 = [LostArkAudioCodec]::Sha256File($mediaFile.FullName)
        origins = @($mediaOrigins[$mediaId] | Sort-Object)
    })
}

$manifest = [ordered]@{
    formatVersion = 1
    generatedAtKst = [DateTimeOffset]::Now.ToString('o')
    source = [ordered]@{
        pckCount = $pckFiles.Count
        wemCount = $wemFiles.Count
        selectedPckCount = $selectedNames.Count
    }
    key = [ordered]@{
        byteCount = $key.Length
        sha256 = [LostArkAudioCodec]::Sha256($key)
    }
    packages = @($packageResults)
    media = @($mediaResults)
}
$manifestPath = Join-Path $WorkRoot 'StageManifest.json'
$manifestJson = $manifest | ConvertTo-Json -Depth 12
[System.IO.File]::WriteAllText($manifestPath, $manifestJson + [Environment]::NewLine, [System.Text.UTF8Encoding]::new($false))

if (Get-ChildItem -LiteralPath $ScratchRoot -Force | Select-Object -First 1) {
    throw "Scratch directory is not empty after staging: $ScratchRoot"
}
Remove-Item -LiteralPath $ScratchRoot

Write-Host "Stage complete."
Write-Host "  WorkRoot: $WorkRoot"
Write-Host "  Banks:    $((Get-ChildItem -LiteralPath $BankRoot -Recurse -File -Filter '*.bnk').Count)"
Write-Host "  Media:    $($mediaResults.Count)"
Write-Host "  Manifest: $manifestPath"
