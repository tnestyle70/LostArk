param(
    [Parameter(Mandatory=$true)][string]$OutputDirectory,
    [string]$RepoRoot
)
$ErrorActionPreference='Stop'
if(!$RepoRoot){$RepoRoot=Split-Path -Parent (Split-Path -Parent $PSScriptRoot)}
$repo=(Resolve-Path -LiteralPath $RepoRoot).Path
$destination=$ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($OutputDirectory)
if(Test-Path -LiteralPath $destination){throw "Corpus output already exists; preserve frozen input: $destination"}
$utf8=[Text.UTF8Encoding]::new($false)
$baseline=Join-Path $destination 'baseline'
New-Item -ItemType Directory -Force $baseline,(Join-Path $destination 'corpus')|Out-Null
$sources=@(foreach($relative in @('Client/Public/DataJson.h','Client/Private/DataJson.cpp','Client/Public/Client_Defines.h')){
    $original=Join-Path $repo $relative
    $snapshot=Join-Path $baseline ([IO.Path]::GetFileName($relative))
    $hash=(Get-FileHash -LiteralPath $original).Hash.ToLowerInvariant()
    Copy-Item -LiteralPath $original -Destination $snapshot
    if($hash -ne (Get-FileHash -LiteralPath $snapshot).Hash.ToLowerInvariant() -or $hash -ne (Get-FileHash -LiteralPath $original).Hash.ToLowerInvariant()){throw 'Source changed during snapshot.'}
    [ordered]@{path=$relative;sha256=$hash}
})
$inputs=@('Data/Animation/Authored/KoukuSaydon/KoukuSaydon.patternbindings.json','Data/Compositions/Sequences/KoukuSaydonSequenceComposition.json','Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.worldsequences.json','Data/Effects/EffectCatalog.json')
$dependencies=@($inputs|ForEach-Object{[ordered]@{path=$_;sha256=(Get-FileHash -LiteralPath (Join-Path $repo $_)).Hash.ToLowerInvariant()}})
$docs=@($inputs|ForEach-Object{Get-Content -LiteralPath (Join-Path $repo $_) -Encoding UTF8 -Raw|ConvertFrom-Json})
$ids=[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach($id in @('effect.kouku.card.match.bind.floor','effect.kouku.card.match.bind.release','effect.kouku.mario.marker.red','effect.kouku.mario.marker.blue','effect.kouku.mario.marker.yellow','effect.kouku.mario.ball.pop.red','effect.kouku.mario.ball.pop.blue','effect.kouku.mario.ball.pop.yellow','effect.kouku.mario.flyingball.hit')){$null=$ids.Add($id)}
function Add-Resource($row){if($row.kind -eq 'EFFECT' -and $row.resourceKind -in @('V1_EFFECT','V1_ELEMENT')){$null=$ids.Add([string]$row.assetId)}}
foreach($pattern in $docs[0].patterns){foreach($resource in $pattern.presentationOccurrences){Add-Resource $resource}}
foreach($fear in $docs[0].fearPresentations){if($fear.effectResource){Add-Resource $fear.effectResource}}
foreach($visual in $docs[0].targetedCombatVisuals){foreach($resource in $visual.resources){Add-Resource $resource};if($visual.contactEffectAssetId){$null=$ids.Add([string]$visual.contactEffectAssetId)}}
$used=[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach($pattern in $docs[1].patterns){foreach($resource in $pattern.presentationOccurrences){$null=$used.Add([string]$resource.resourceId)}}
foreach($resource in $docs[1].presentationResources){if($used.Remove([string]$resource.resourceId)){Add-Resource $resource}}
if($used.Count){throw 'Unresolved sequence resource.'}
$templates=[Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal)
foreach($instance in $docs[2].instances){if($instance.enabled){$null=$templates.Add([string]$instance.templateId)}}
foreach($template in $docs[2].templates){if($templates.Remove([string]$template.sequenceId)){foreach($effect in $template.effectTracks){if($effect.resourceKind -in @('V1_EFFECT','V1_ELEMENT')){$null=$ids.Add([string]$effect.resourceId)}}}}
if($templates.Count){throw 'Unresolved world sequence template.'}
$catalog=@{}
foreach($entry in $docs[3].effects){$catalog[[string]$entry.effectAssetId]=$entry}
$files=@();$lines=@();$index=0
$dataRoot=[IO.Path]::GetFullPath((Join-Path $repo 'Data'))+[IO.Path]::DirectorySeparatorChar
foreach($id in @($ids|Sort-Object)){
    if(!$catalog.ContainsKey($id)){throw "Missing effect catalog ID: $id"}
    $entry=$catalog[$id]
    if($entry.payloadKind -ne 'DIRECT_AUTHORED_DOCUMENT'){throw "Unexpected payload kind: $id"}
    $source=[IO.Path]::GetFullPath((Join-Path $dataRoot $entry.authoringPath))
    if(!$source.StartsWith($dataRoot,[StringComparison]::OrdinalIgnoreCase)){throw "Authoring path outside Data: $id"}
    $relative='corpus/{0:D3}.json' -f $index++
    $snapshot=Join-Path $destination $relative
    $before=(Get-FileHash -LiteralPath $source).Hash.ToLowerInvariant()
    Copy-Item -LiteralPath $source -Destination $snapshot
    $after=(Get-FileHash -LiteralPath $source).Hash.ToLowerInvariant()
    $copy=(Get-FileHash -LiteralPath $snapshot).Hash.ToLowerInvariant()
    if($before -ne $after -or $copy -ne $before){throw "Concurrent source mutation: $id"}
    $files += [pscustomobject][ordered]@{effectAssetId=$id;sourcePath=$source;snapshotPath=$relative;bytes=(Get-Item -LiteralPath $snapshot).Length;sha256=$copy}
    $lines += "$id`t$relative"
}
foreach($dependency in $dependencies){if($dependency.sha256 -ne (Get-FileHash -LiteralPath (Join-Path $repo $dependency.path)).Hash.ToLowerInvariant()){throw 'Dependency changed while preparing corpus.'}}
$manifest=[ordered]@{gitRef=(& git -C $repo rev-parse HEAD);utc=[DateTime]::UtcNow.ToString('o');corpusBoundary='KoukuSaydon saved V1 closure matching Collect_ProductEffectTargets branches; excludes Valtan, V2, actor/GPU preparation and live unsaved draft';sourceHashes=$sources;dependencyInputs=$dependencies;count=$files.Count;bytes=($files|Measure-Object -Property bytes -Sum).Sum;files=$files}
[IO.File]::WriteAllText((Join-Path $destination 'corpus-manifest.json'),($manifest|ConvertTo-Json -Depth 8),$utf8)
[IO.File]::WriteAllText((Join-Path $destination 'corpus.tsv'),($lines -join "`n")+"`n",$utf8)
[pscustomobject]$manifest|Select-Object gitRef,count,bytes|ConvertTo-Json
