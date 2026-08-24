function ConvertTo-FormatPreservingCanonicalNode([object]$Value) {
    if ($null -eq $Value) { return $null }
    if ($Value -is [string] -or $Value -is [bool] -or
        $Value -is [byte] -or $Value -is [sbyte] -or
        $Value -is [int16] -or $Value -is [uint16] -or
        $Value -is [int32] -or $Value -is [uint32] -or
        $Value -is [int64] -or $Value -is [uint64] -or
        $Value -is [single] -or $Value -is [double] -or
        $Value -is [decimal]) {
        return $Value
    }
    if ($Value -is [Collections.IDictionary]) {
        $ordered = [ordered]@{}
        [string[]]$keys = @($Value.Keys | ForEach-Object { [string]$_ })
        [Array]::Sort($keys, [StringComparer]::Ordinal)
        foreach ($key in $keys) {
            $ordered[$key] = ConvertTo-FormatPreservingCanonicalNode $Value[$key]
        }
        return [pscustomobject]$ordered
    }
    if ($Value -is [Collections.IEnumerable] -and $Value -isnot [string]) {
        $items = @()
        foreach ($item in $Value) {
            $items += ,(ConvertTo-FormatPreservingCanonicalNode $item)
        }
        return ,$items
    }
    $object = [ordered]@{}
    [string[]]$names = @($Value.PSObject.Properties.Name)
    [Array]::Sort($names, [StringComparer]::Ordinal)
    foreach ($name in $names) {
        $object[$name] = ConvertTo-FormatPreservingCanonicalNode $Value.$name
    }
    return [pscustomobject]$object
}

function Test-FormatPreservingSemanticEquality([object]$Left, [object]$Right) {
    $leftText = ConvertTo-FormatPreservingCanonicalNode $Left |
        ConvertTo-Json -Depth 100 -Compress
    $rightText = ConvertTo-FormatPreservingCanonicalNode $Right |
        ConvertTo-Json -Depth 100 -Compress
    return $leftText -ceq $rightText
}

function Get-FormatPreservingExactProperty(
    [object]$Value,
    [string]$PropertyName,
    [string]$Context) {
    $matches = @($Value.PSObject.Properties | Where-Object {
        [string]$_.Name -ceq $PropertyName
    })
    if ($matches.Count -ne 1) {
        throw "$Context is missing exact property '$PropertyName'."
    }
    return $matches[0]
}

function Get-FormatPreservingNewline([string]$Text) {
    if ($Text.Contains("`r`n")) { return "`r`n" }
    return "`n"
}

function Get-FormatPreservingLineIndent([string]$Text, [int]$Index) {
    $lineBreak = $Text.LastIndexOf("`n", [math]::Max(0, $Index - 1))
    $lineStart = if ($lineBreak -lt 0) { 0 } else { $lineBreak + 1 }
    $indent = $Text.Substring($lineStart, $Index - $lineStart)
    if ($indent -notmatch '^\s*$') {
        return ''
    }
    return $indent
}

function Get-FormatPreservingRootArrayProperty(
    [string]$Text,
    [string]$ArrayProperty) {
    $matches = [Collections.Generic.List[object]]::new()
    $depth = 0
    for ($offset = 0; $offset -lt $Text.Length; ++$offset) {
        $character = $Text[$offset]
        if ($character -eq '"') {
            $stringStart = $offset
            $escaped = $false
            for (++$offset; $offset -lt $Text.Length; ++$offset) {
                $stringCharacter = $Text[$offset]
                if ($escaped) { $escaped = $false }
                elseif ($stringCharacter -eq '\') { $escaped = $true }
                elseif ($stringCharacter -eq '"') { break }
            }
            if ($offset -ge $Text.Length) {
                throw 'JSON document has an unterminated string.'
            }
            if ($depth -ne 1) { continue }
            $afterString = $offset + 1
            while ($afterString -lt $Text.Length -and
                [char]::IsWhiteSpace($Text[$afterString])) {
                ++$afterString
            }
            if ($afterString -ge $Text.Length -or $Text[$afterString] -ne ':') {
                continue
            }
            $propertyToken = $Text.Substring(
                $stringStart,
                $offset - $stringStart + 1)
            try { $propertyName = $propertyToken | ConvertFrom-Json }
            catch { throw 'JSON document has an invalid root property string.' }
            if ([string]$propertyName -cne $ArrayProperty) { continue }
            $valueOffset = $afterString + 1
            while ($valueOffset -lt $Text.Length -and
                [char]::IsWhiteSpace($Text[$valueOffset])) {
                ++$valueOffset
            }
            if ($valueOffset -ge $Text.Length -or $Text[$valueOffset] -ne '[') {
                throw "Root JSON property '$ArrayProperty' is not an array."
            }
            $matches.Add([pscustomobject]@{
                PropertyOffset = $stringStart
                OpenOffset = $valueOffset
            })
            continue
        }
        if ($character -eq '{' -or $character -eq '[') {
            ++$depth
        }
        elseif ($character -eq '}' -or $character -eq ']') {
            --$depth
            if ($depth -lt 0) { throw 'JSON document has an unmatched close.' }
        }
    }
    if ($depth -ne 0) { throw 'JSON document is unbalanced.' }
    if ($matches.Count -ne 1) {
        throw "Expected exactly one root JSON array property '$ArrayProperty'."
    }
    return $matches[0]
}

function Get-FormatPreservingJsonArrayLayout(
    [string]$Text,
    [string]$ArrayProperty,
    [string]$KeyProperty) {
    $propertyMatch = Get-FormatPreservingRootArrayProperty $Text $ArrayProperty
    $openOffset = [int]$propertyMatch.OpenOffset

    $elements = [Collections.Generic.List[object]]::new()
    $depth = 1
    $inString = $false
    $escaped = $false
    $elementStart = -1
    $closeOffset = -1
    for ($offset = $openOffset + 1; $offset -lt $Text.Length; ++$offset) {
        $character = $Text[$offset]
        if ($inString) {
            if ($escaped) {
                $escaped = $false
            }
            elseif ($character -eq '\') {
                $escaped = $true
            }
            elseif ($character -eq '"') {
                $inString = $false
            }
            continue
        }
        if ($character -eq '"') {
            $inString = $true
            continue
        }
        if ($character -eq '{' -or $character -eq '[') {
            if ($depth -eq 1 -and $character -eq '{') { $elementStart = $offset }
            ++$depth
            continue
        }
        if ($character -eq '}' -or $character -eq ']') {
            if ($character -eq ']' -and $depth -eq 1) {
                $closeOffset = $offset
                break
            }
            --$depth
            if ($depth -lt 1) { throw "JSON array '$ArrayProperty' is unbalanced." }
            if ($character -eq '}' -and $depth -eq 1) {
                if ($elementStart -lt 0) {
                    throw "JSON array '$ArrayProperty' has an unmatched object close."
                }
                $raw = $Text.Substring($elementStart, $offset - $elementStart + 1)
                try { $row = $raw | ConvertFrom-Json }
                catch { throw "JSON array '$ArrayProperty' contains an invalid object row." }
                $keyPropertyValue = Get-FormatPreservingExactProperty `
                    $row $KeyProperty "JSON array '$ArrayProperty' row"
                if ($keyPropertyValue.Value -isnot [string] -or
                    [string]::IsNullOrWhiteSpace([string]$keyPropertyValue.Value)) {
                    throw "JSON array '$ArrayProperty' row is missing '$KeyProperty'."
                }
                $elements.Add([pscustomobject]@{
                    Key = [string]$keyPropertyValue.Value
                    Start = $elementStart
                    End = $offset
                    Raw = $raw
                    Value = $row
                    OriginalIndex = $elements.Count
                })
                $elementStart = -1
            }
        }
    }
    if ($closeOffset -lt 0 -or $depth -ne 1 -or $inString) {
        throw "JSON array '$ArrayProperty' is unterminated."
    }

    $keys = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($element in @($elements)) {
        if (-not $keys.Add([string]$element.Key)) {
            throw "JSON array '$ArrayProperty' has duplicate $KeyProperty '$($element.Key)'."
        }
    }

    $propertyIndent = Get-FormatPreservingLineIndent `
        $Text ([int]$propertyMatch.PropertyOffset)
    $arrayText = $Text.Substring($openOffset, $closeOffset - $openOffset + 1)
    $newline = Get-FormatPreservingNewline $arrayText
    $itemIndent = if ($elements.Count -gt 0) {
        Get-FormatPreservingLineIndent $Text $elements[0].Start
    }
    else {
        $propertyIndent + '  '
    }
    $indentUnit = if ($itemIndent.StartsWith($propertyIndent) -and
        $itemIndent.Length -gt $propertyIndent.Length) {
        $itemIndent.Substring($propertyIndent.Length)
    }
    else {
        '  '
    }
    $leadingEnd = if ($elements.Count -gt 0) { $elements[0].Start } else { $closeOffset }
    $leadingTrivia = $Text.Substring($openOffset + 1, $leadingEnd - $openOffset - 1)
    $trailingStart = if ($elements.Count -gt 0) {
        $elements[$elements.Count - 1].End + 1
    }
    else {
        $closeOffset
    }
    $trailingTrivia = $Text.Substring($trailingStart, $closeOffset - $trailingStart)
    $separators = [Collections.Generic.List[string]]::new()
    for ($index = 0; $index + 1 -lt $elements.Count; ++$index) {
        $separatorStart = $elements[$index].End + 1
        $separatorEnd = $elements[$index + 1].Start
        $separator = $Text.Substring($separatorStart, $separatorEnd - $separatorStart)
        if ($separator -notmatch ',') {
            throw "JSON array '$ArrayProperty' has an invalid row separator."
        }
        $separators.Add($separator)
    }
    return [pscustomobject]@{
        OpenOffset = $openOffset
        CloseOffset = $closeOffset
        Elements = @($elements)
        Separators = @($separators)
        LeadingTrivia = $leadingTrivia
        TrailingTrivia = $trailingTrivia
        PropertyIndent = $propertyIndent
        ItemIndent = $itemIndent
        IndentUnit = $indentUnit
        Newline = $newline
    }
}

function ConvertTo-FormatPreservingPrettyJson([object]$Value, [string]$Newline) {
    $compact = $Value | ConvertTo-Json -Depth 100 -Compress
    $builder = [Text.StringBuilder]::new()
    $expandedContainers = [Collections.Generic.Stack[bool]]::new()
    $depth = 0
    $inString = $false
    $escaped = $false
    for ($index = 0; $index -lt $compact.Length; ++$index) {
        $character = $compact[$index]
        if ($inString) {
            $null = $builder.Append($character)
            if ($escaped) { $escaped = $false }
            elseif ($character -eq '\') { $escaped = $true }
            elseif ($character -eq '"') { $inString = $false }
            continue
        }
        if ($character -eq '"') {
            $inString = $true
            $null = $builder.Append($character)
            continue
        }
        if ($character -eq '{' -or $character -eq '[') {
            $null = $builder.Append($character)
            $matchingClose = if ($character -eq '{') { '}' } else { ']' }
            $expanded = $index + 1 -lt $compact.Length -and
                $compact[$index + 1] -ne $matchingClose
            $expandedContainers.Push($expanded)
            if ($expanded) {
                ++$depth
                $null = $builder.Append($Newline)
                $null = $builder.Append('  ' * $depth)
            }
            continue
        }
        if ($character -eq '}' -or $character -eq ']') {
            if ($expandedContainers.Count -eq 0) {
                throw 'Compact JSON formatter encountered an unmatched close.'
            }
            if ($expandedContainers.Pop()) {
                --$depth
                $null = $builder.Append($Newline)
                $null = $builder.Append('  ' * $depth)
            }
            $null = $builder.Append($character)
            continue
        }
        if ($character -eq ',') {
            $null = $builder.Append(',')
            $null = $builder.Append($Newline)
            $null = $builder.Append('  ' * $depth)
            continue
        }
        if ($character -eq ':') {
            $null = $builder.Append(': ')
            continue
        }
        if (-not [char]::IsWhiteSpace($character)) {
            $null = $builder.Append($character)
        }
    }
    if ($inString -or $expandedContainers.Count -ne 0 -or $depth -ne 0) {
        throw 'Compact JSON formatter produced an unbalanced document.'
    }
    return $builder.ToString()
}

function ConvertTo-FormatPreservingJsonArrayElement(
    [object]$Value,
    [string]$ItemIndent,
    [string]$IndentUnit,
    [string]$Newline) {
    $raw = ConvertTo-FormatPreservingPrettyJson $Value $Newline
    [string[]]$lines = @($raw -split '\r?\n')
    $formatted = [Collections.Generic.List[string]]::new()
    for ($lineIndex = 0; $lineIndex -lt $lines.Count; ++$lineIndex) {
        $line = $lines[$lineIndex]
        $trimmed = $line.TrimStart(' ')
        if ($lineIndex -eq 0) {
            $formatted.Add($trimmed)
            continue
        }
        $leadingSpaces = $line.Length - $trimmed.Length
        $level = [int][math]::Floor($leadingSpaces / 2)
        $formatted.Add($ItemIndent + ($IndentUnit * $level) + $trimmed)
    }
    return $formatted -join $Newline
}

function Update-FormatPreservingJsonArrayRows(
    [string]$Text,
    [string]$ArrayProperty,
    [string]$KeyProperty,
    [object[]]$DesiredRows,
    [string[]]$RemoveKeys = @(),
    [switch]$InsertMissingAtFirstRemoval,
    [switch]$InsertMissingByDesiredOrder) {
    $layout = Get-FormatPreservingJsonArrayLayout $Text $ArrayProperty $KeyProperty
    $desiredByKey =
        [Collections.Generic.Dictionary[string,object]]::new([StringComparer]::Ordinal)
    $desiredOrder = [Collections.Generic.List[string]]::new()
    foreach ($row in @($DesiredRows)) {
        $property = Get-FormatPreservingExactProperty `
            $row $KeyProperty "Desired '$ArrayProperty' row"
        if ($property.Value -isnot [string] -or
            [string]::IsNullOrWhiteSpace([string]$property.Value)) {
            throw "Desired '$ArrayProperty' row is missing '$KeyProperty'."
        }
        $key = [string]$property.Value
        if ($desiredByKey.ContainsKey($key)) {
            throw "Desired '$ArrayProperty' rows contain duplicate '$key'."
        }
        $desiredByKey[$key] = $row
        $desiredOrder.Add($key)
    }
    $removeSet = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($key in @($RemoveKeys)) { $null = $removeSet.Add([string]$key) }
    foreach ($key in @($desiredByKey.Keys)) {
        if ($removeSet.Contains([string]$key)) {
            throw "JSON array '$ArrayProperty' cannot replace and remove '$key'."
        }
    }

    $existingKeys = [Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
    foreach ($element in @($layout.Elements)) { $null = $existingKeys.Add([string]$element.Key) }
    $missingKeys = [Collections.Generic.List[string]]::new()
    foreach ($key in @($desiredOrder)) {
        if (-not $existingKeys.Contains([string]$key)) { $missingKeys.Add([string]$key) }
    }

    $resultElements = [Collections.Generic.List[object]]::new()
    $insertedMissing = $false
    foreach ($element in @($layout.Elements)) {
        if ($removeSet.Contains([string]$element.Key)) {
            if ($InsertMissingAtFirstRemoval -and -not $insertedMissing) {
                foreach ($missingKey in @($missingKeys)) {
                    $resultElements.Add([pscustomobject]@{
                        Key = $missingKey
                        OriginalIndex = -1
                        Raw = ConvertTo-FormatPreservingJsonArrayElement `
                            $desiredByKey[$missingKey] $layout.ItemIndent `
                            $layout.IndentUnit $layout.Newline
                    })
                }
                $insertedMissing = $true
            }
            continue
        }
        $raw = [string]$element.Raw
        if ($desiredByKey.ContainsKey([string]$element.Key) -and
            -not (Test-FormatPreservingSemanticEquality `
                $element.Value $desiredByKey[[string]$element.Key])) {
            $raw = ConvertTo-FormatPreservingJsonArrayElement `
                $desiredByKey[[string]$element.Key] $layout.ItemIndent `
                $layout.IndentUnit $layout.Newline
        }
        $resultElements.Add([pscustomobject]@{
            Key = [string]$element.Key
            OriginalIndex = [int]$element.OriginalIndex
            Raw = $raw
        })
    }
    if (-not $insertedMissing) {
        foreach ($missingKey in @($missingKeys)) {
            $missingElement = [pscustomobject]@{
                Key = $missingKey
                OriginalIndex = -1
                Raw = ConvertTo-FormatPreservingJsonArrayElement `
                    $desiredByKey[$missingKey] $layout.ItemIndent `
                    $layout.IndentUnit $layout.Newline
            }
            if ($InsertMissingByDesiredOrder) {
                $missingDesiredIndex = $desiredOrder.IndexOf([string]$missingKey)
                $insertIndex = $resultElements.Count
                for ($candidateIndex = 0;
                    $candidateIndex -lt $resultElements.Count;
                    ++$candidateIndex) {
                    $candidateDesiredIndex =
                        $desiredOrder.IndexOf([string]$resultElements[$candidateIndex].Key)
                    if ($candidateDesiredIndex -gt $missingDesiredIndex) {
                        $insertIndex = $candidateIndex
                        break
                    }
                }
                $resultElements.Insert($insertIndex, $missingElement)
            }
            else {
                $resultElements.Add($missingElement)
            }
        }
    }

    $defaultSeparator = ',' + $layout.Newline + $layout.ItemIndent
    $body = [Text.StringBuilder]::new()
    $null = $body.Append($layout.LeadingTrivia)
    for ($index = 0; $index -lt $resultElements.Count; ++$index) {
        $current = $resultElements[$index]
        $null = $body.Append([string]$current.Raw)
        if ($index + 1 -ge $resultElements.Count) { continue }
        $next = $resultElements[$index + 1]
        $separator = $defaultSeparator
        if ([int]$current.OriginalIndex -ge 0 -and
            [int]$next.OriginalIndex -eq [int]$current.OriginalIndex + 1) {
            $separator = [string]$layout.Separators[[int]$current.OriginalIndex]
        }
        elseif ([int]$current.OriginalIndex -ge 0 -and
            [int]$current.OriginalIndex -lt $layout.Separators.Count) {
            $separator = [string]$layout.Separators[[int]$current.OriginalIndex]
        }
        elseif ([int]$next.OriginalIndex -gt 0) {
            $separator = [string]$layout.Separators[[int]$next.OriginalIndex - 1]
        }
        $null = $body.Append($separator)
    }
    $null = $body.Append($layout.TrailingTrivia)
    return $Text.Substring(0, $layout.OpenOffset + 1) + $body.ToString() +
        $Text.Substring($layout.CloseOffset)
}
