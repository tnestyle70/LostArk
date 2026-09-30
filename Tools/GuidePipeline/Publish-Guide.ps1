[CmdletBinding()]
param(
    [ValidateSet('Validate','Publish','CheckPublished','Save')][string]$Mode = 'Validate',
    [string]$RepositoryRoot = '',
    [string]$BaselinePath = '', [string]$DraftPath = ''
)
$ErrorActionPreference = 'Stop'
if (-not $RepositoryRoot) { $RepositoryRoot = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent }
Set-StrictMode -Version Latest
. (Join-Path $RepositoryRoot 'Tools/GameplayPipeline/Publish-FileTransaction.ps1')
$utf8 = [Text.UTF8Encoding]::new($false, $true)
$sources = @{}
$files = [ordered]@{catalog='GuideCatalog.json';placement='DimensionMaster/Placement.json';prompts='DimensionMaster/Prompts.json';triggers='DimensionMaster/Triggers.json';combat='DimensionMaster/Combat.json'}
function Assert-RepositoryPath([string]$path) {
    $root=[IO.Path]::GetFullPath($RepositoryRoot).TrimEnd([IO.Path]::DirectorySeparatorChar)
    $full=[IO.Path]::GetFullPath($path)
    Assert-True ($full.StartsWith($root+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)) "Guide path escapes repository: $path"
    $check=$full
    while($check.Length -gt $root.Length){
        if([IO.File]::Exists($check) -or [IO.Directory]::Exists($check)){
            Assert-True (([IO.File]::GetAttributes($check) -band [IO.FileAttributes]::ReparsePoint) -eq 0) "Guide path uses a reparse point: $check"
        }
        $check=[IO.Path]::GetDirectoryName($check)
    }
}
function Read-Json([string]$path,[bool]$repositoryOwned=$true) {
    if($repositoryOwned){Assert-RepositoryPath $path}
    $bytes=[IO.File]::ReadAllBytes($path); $sources[$path]=[Convert]::ToBase64String($bytes)
    $text=$utf8.GetString($bytes).TrimStart([char]0xFEFF)
    $parsed=ConvertFrom-Json -InputObject $text
    # ConvertFrom-Json otherwise silently retains the last exact duplicate key.
    $objects=[Collections.Generic.List[object]]::new(); $previous=''
    foreach($token in [Regex]::Matches($text, '"(?:\\.|[^"\\])*"|[{}\[\]:]')){
        switch -CaseSensitive ($token.Value){
            '{' {$objects.Add([Collections.Generic.HashSet[string]]::new([StringComparer]::Ordinal))}
            '[' {$objects.Add($null)}
            '}' {$objects.RemoveAt($objects.Count-1)}
            ']' {$objects.RemoveAt($objects.Count-1)}
            ':' {
                $key=(ConvertFrom-Json -InputObject ('{"key":'+$previous+'}')).key
                Assert-True ($objects[$objects.Count-1].Add($key)) "Duplicate JSON property '$key' in $path"
            }
            default {$previous=$token.Value}
        }
    }
    return ,$parsed
}
function Assert-True($condition,[string]$message) { if (-not $condition) { throw $message } }
function Assert-Keys($value,[string[]]$keys,[string]$where) {
    Assert-True ($null -ne $value -and $value -is [pscustomobject]) "$where must be an object"
    $actual=@($value.PSObject.Properties.Name)
    Assert-True (@($actual | Where-Object {$_ -cnotin $keys}).Count -eq 0 -and @($keys | Where-Object {$_ -cnotin $actual}).Count -eq 0) "$where properties differ from its schema"
}
function Assert-Number($value,[double]$minimum,[double]$maximum,[string]$where) {
    Assert-True ($value -is [ValueType] -and $value -isnot [bool] -and [double]::IsNaN([double]$value) -eq $false -and [double]::IsInfinity([double]$value) -eq $false -and $value -ge $minimum -and $value -le $maximum) "$where is outside [$minimum,$maximum]"
}
function Assert-Integer($value,[double]$minimum,[double]$maximum,[string]$where) {
    Assert-Number $value $minimum $maximum $where
    Assert-True ([Math]::Floor([double]$value) -eq [double]$value) "$where requires a whole number"
}
function Assert-Id($id,[string]$where) { Assert-True ($id -is [string] -and $id -cmatch '^[A-Za-z0-9_.:-]{1,128}$') "$where requires an ASCII stable ID" }
function Assert-Text($text,[int]$max,[string]$where) { Assert-True ($text -is [string] -and $text.Length -gt 0 -and $utf8.GetByteCount($text) -le $max -and $text -notmatch '[\x00-\x08\x0b-\x1f\x7f]') "$where text is empty or too long" }
function Assert-Vector($value,[double]$minimum,[string]$where) { Assert-True ($value -is [array] -and $value.Count -eq 3) "$where requires three coordinates"; foreach($n in $value){ Assert-Number $n $minimum 100000 $where } }
function Canonical-Value($value) {
    if($value -is [pscustomobject] -or $value -is [Collections.IDictionary]){
        $result=[ordered]@{}
        $names=if($value -is [Collections.IDictionary]){@($value.Keys)}else{@($value.PSObject.Properties.Name)}
        foreach($name in @($names | Sort-Object -CaseSensitive)){ $result[$name]=Canonical-Value $value.$name }
        return ,[pscustomobject]$result
    }
    if($value -is [array]){ $result=[Collections.Generic.List[object]]::new(); foreach($item in $value){$result.Add((Canonical-Value $item))}; return ,$result.ToArray() }
    return ,$value
}
function Json($value) { ConvertTo-Json -InputObject (Canonical-Value $value) -Depth 80 -Compress }
function Clone($value) { ConvertFrom-Json -InputObject (Json $value) }
function Read-Bundle {
    $bundle=[ordered]@{}
    foreach($key in $files.Keys){ $bundle[$key]=Read-Json (Join-Path $RepositoryRoot ('Data/Guide/'+$files[$key])) }
    [pscustomobject]$bundle
}
function Make-Runtime($b) {
    Assert-Keys $b @('catalog','placement','prompts','triggers','combat') 'Guide bundle'
    Assert-Keys $b.catalog @('schema','formatVersion','revision','categories','guides') 'Catalog'
    Assert-Keys $b.placement @('schema','formatVersion','revision','guideId','placementId','categoryId','position','yawDegrees','anchorPolicy') 'Placement'
    Assert-Keys $b.prompts @('schema','formatVersion','revision','guideId','prompts') 'Prompts'
    Assert-Keys $b.triggers @('schema','formatVersion','revision','guideId','triggers') 'Triggers'
    Assert-Keys $b.combat @('schema','formatVersion','revision','guideId','follow','decision','weights','combos','commands') 'Combat'
    foreach($key in $files.Keys){ $doc=$b.$key; $schema=if($key -eq 'catalog'){'lostark.guide-catalog'}else{'lostark.guide-'+$key}; Assert-True ($doc.schema -ceq $schema -and $doc.formatVersion -eq 1) "$key schema/version mismatch"; Assert-Integer $doc.revision 1 9007199254740991 "$key revision" }
    Assert-True (@($b.catalog.guides).Count -eq 1) 'Exactly one guide definition is supported'
    $g=$b.catalog.guides[0]
    Assert-Keys $g @('guideId','characterClass','displayName','placementDocument','promptDocument','triggerDocument','combatDocument') 'Guide definition'
    Assert-True ($g.guideId -ceq 'guide.dimensionmaster' -and $g.characterClass -ceq 'DIMENSIONMASTER') 'Guide must use DIMENSIONMASTER'
    Assert-Text $g.displayName 32 'Guide name'; Assert-True ($g.displayName -notmatch '[\x00-\x1f\x7f]') 'Guide name contains a control character'
    foreach($pair in @(@('placementDocument','placement'),@('promptDocument','prompts'),@('triggerDocument','triggers'),@('combatDocument','combat'))){ Assert-True ($g.($pair[0]) -ceq $files[$pair[1]]) 'Guide source reference differs from the supported document path'; Assert-True ($b.($pair[1]).guideId -ceq $g.guideId) 'Guide document identity mismatch' }
    $map=Read-Json (Join-Path $RepositoryRoot 'Data/Maps/MapCatalog.json')
    $cats=@{}; $worlds=@{}
    foreach($c in $b.catalog.categories){ Assert-Keys $c @('categoryId','displayName','worldId','areaId') 'Category'; Assert-Id $c.categoryId 'Category'; Assert-Text $c.displayName 96 'Category name'; Assert-True (-not $cats.ContainsKey($c.categoryId)) 'Duplicate category'; Assert-True (@($map.areas | Where-Object id -CEQ $c.areaId).Count -eq 1) "Unknown Area $($c.areaId)"; Assert-True ($c.worldId -cin @('BERN','VALTAN_ARENA','KAKULSAYDON_ARENA')) 'Unsupported Guide world'; $cats[$c.categoryId]=$c; $worlds[$c.categoryId]=Read-Json (Join-Path $RepositoryRoot ('Data/Worlds/'+$c.areaId+'/Gameplay.world.json')) }
    Assert-True ($cats.ContainsKey($b.placement.categoryId)) 'Unknown placement category'
    Assert-Id $b.placement.placementId 'Placement'; Assert-Vector $b.placement.position -100000 'Start position'; Assert-Number $b.placement.yawDegrees -36000 36000 'Start yaw'; Assert-True ($b.placement.anchorPolicy -cin @('INVITER_THEN_LEADER','PARTY_LEADER')) 'Unknown anchor policy'
    $promptIds=@{}; Assert-True (@($b.prompts.prompts).Count -le 512) 'Too many prompts'
    foreach($p in $b.prompts.prompts){ Assert-Keys $p @('promptId','categoryId','topic','title','segments') 'Prompt'; Assert-Id $p.promptId 'Prompt'; Assert-True (-not $promptIds.ContainsKey($p.promptId) -and $cats.ContainsKey($p.categoryId)) 'Duplicate prompt or unknown category'; $promptIds[$p.promptId]=$true; Assert-Text $p.title 256 'Prompt title'; Assert-True ($p.topic -cin @('PARTY','NPC','BOSS_PATTERN','COMBAT')) 'Unknown prompt topic'; Assert-True (@($p.segments).Count -ge 1 -and @($p.segments).Count -le 16) 'Prompt needs 1..16 segments'; $bytes=0; $duration=0; foreach($s in $p.segments){ Assert-Keys $s @('text','durationMs') 'Prompt segment'; Assert-Text $s.text 512 'Segment'; Assert-Integer $s.durationMs 1000 20000 'Segment duration'; $bytes+=$utf8.GetByteCount($s.text); $duration+=$s.durationMs }; Assert-True ($bytes -le 4096 -and $duration -le 60000) 'Prompt exceeds text or duration limit' }
    $f=$b.combat.follow; Assert-Keys $f @('desiredDistanceM','minimumDistanceM','maximumDistanceM','resumeDistanceM','recoverDistanceM','recoverDelayMs') 'Follow'; foreach($k in @('desiredDistanceM','minimumDistanceM','maximumDistanceM','resumeDistanceM','recoverDistanceM')){ Assert-Number $f.$k 0.1 200 "Follow $k" }; Assert-True ($f.minimumDistanceM -le $f.desiredDistanceM -and $f.desiredDistanceM -le $f.maximumDistanceM -and $f.maximumDistanceM -lt $f.resumeDistanceM -and $f.resumeDistanceM -lt $f.recoverDistanceM) 'Follow distances are not ordered'; Assert-Integer $f.recoverDelayMs 1000 60000 'Recovery delay'
    $d=$b.combat.decision; Assert-Keys $d @('thinkIntervalMs','horizonMs','switchMargin','minimumHoldMs','lethalHpFraction') 'Decision'; Assert-Integer $d.thinkIntervalMs 33 1000 'Think interval'; Assert-Integer $d.horizonMs 100 5000 'Horizon'; Assert-Number $d.switchMargin 0 1 'Switch margin'; Assert-Integer $d.minimumHoldMs 0 3000 'Minimum hold'; Assert-Number $d.lethalHpFraction 0.1 1 'Lethal HP fraction'
    Assert-Keys $b.combat.weights @('FOLLOW','ASSIST') 'Weights'; foreach($mode in @('FOLLOW','ASSIST')){ $w=$b.combat.weights.$mode; Assert-Keys $w @('attack','avoid','follow') 'Weight'; foreach($k in @('attack','avoid','follow')){ Assert-Number $w.$k 0 1 "$mode $k" }; Assert-True ([Math]::Abs($w.attack+$w.avoid+$w.follow-1) -lt 0.001) "$mode weights must sum to 1" }; Assert-True ($b.combat.weights.FOLLOW.attack -eq 0) 'FOLLOW mode cannot attack without a help request'
    $skills=Read-Json (Join-Path $RepositoryRoot 'Data/Balance/PlayerSkills.json'); $bootstrapPath=Join-Path $RepositoryRoot 'Server/Bin/DataFiles/Gameplay/Gameplay.bootstrap'; Assert-RepositoryPath $bootstrapPath; $bootstrapBytes=[IO.File]::ReadAllBytes($bootstrapPath); $bootstrap=$utf8.GetString($bootstrapBytes); $sources[$bootstrapPath]=[Convert]::ToBase64String($bootstrapBytes)
    $comboIds=@{}; $combat=Clone $b.combat; Assert-True (@($combat.combos).Count -le 128) 'Too many combos'
    foreach($combo in $combat.combos){ Assert-Keys $combo @('comboId','displayName','inputSlots','timeoutMs','stepWaitMs','repeat') 'Combo'; Assert-Id $combo.comboId 'Combo'; Assert-True (-not $comboIds.ContainsKey($combo.comboId)) 'Duplicate combo'; $comboIds[$combo.comboId]=$true; Assert-Text $combo.displayName 128 'Combo name'; Assert-Integer $combo.timeoutMs 1000 120000 'Combo timeout'; Assert-Integer $combo.stepWaitMs 0 30000 'Step wait'; Assert-True ($combo.repeat -is [bool] -and @($combo.inputSlots).Count -ge 1 -and @($combo.inputSlots).Count -le 32) 'Invalid combo steps/repeat'; $resolved=@(); foreach($slot in $combo.inputSlots){ $matches=@($skills.skills | Where-Object {$_.characterClass -ceq $g.characterClass -and $_.inputSlot -ceq $slot -and $_.skillKind -ceq 'ACTIVE'}); Assert-True ($matches.Count -eq 1) "Unresolvable active skill slot $slot"; $id=$matches[0].skillId; Assert-True ($bootstrap -cmatch "(?m)^SKILL\t$id\tDIMENSIONMASTER\t$slot\t") "Skill $slot is not published"; $resolved += $id }; $combo | Add-Member -NotePropertyName resolvedSkillIds -NotePropertyValue @($resolved) }
    $aliases=@{}; $commandIds=@{}; foreach($command in $b.combat.commands){ Assert-Keys $command @('commandId','displayName','aliases','comboId','stop','enabled','cooldownMs') 'Command'; Assert-Id $command.commandId 'Command'; Assert-True (-not $commandIds.ContainsKey($command.commandId)) 'Duplicate command'; $commandIds[$command.commandId]=$true; Assert-Text $command.displayName 128 'Command name'; Assert-True ($command.stop -is [bool] -and $command.enabled -is [bool]) 'Command flags must be booleans'; Assert-True (($command.stop -and $command.comboId -ceq '') -or (-not $command.stop -and $comboIds.ContainsKey($command.comboId))) 'Command combo reference is invalid'; Assert-Integer $command.cooldownMs 0 60000 'Command cooldown'; Assert-True (@($command.aliases).Count -ge 1 -and @($command.aliases).Count -le 32) 'Command needs aliases'; foreach($alias in $command.aliases){ Assert-Text $alias 128 'Command alias'; Assert-True ($alias -ceq $alias.Trim()) 'Alias must have no surrounding spaces'; if($command.enabled){ Assert-True (-not $aliases.ContainsKey($alias)) "Duplicate enabled command alias: $alias"; $aliases[$alias]=$true } } }
    $triggerIds=@{}; foreach($t in $b.triggers.triggers){ Assert-Keys $t @('triggerId','categoryId','enabled','event','promptId','comboId','cooldownMs','priority') 'Trigger'; Assert-Id $t.triggerId 'Trigger'; Assert-True (-not $triggerIds.ContainsKey($t.triggerId) -and $cats.ContainsKey($t.categoryId)) 'Duplicate trigger or unknown category'; $triggerIds[$t.triggerId]=$true; Assert-True ($t.enabled -is [bool]) 'Trigger enabled must be boolean'; Assert-True ($t.promptId -ceq '' -or $promptIds.ContainsKey($t.promptId)) 'Trigger prompt is missing'; Assert-True ($t.comboId -ceq '' -or $comboIds.ContainsKey($t.comboId)) 'Trigger combo is missing'; Assert-True ($t.event.type -ceq 'HELP_COMMAND' -or $t.comboId -ceq '') 'Only HELP_COMMAND triggers can request a combo'; Assert-Integer $t.cooldownMs 0 3600000 'Trigger cooldown'; Assert-Integer $t.priority 0 255 'Trigger priority'; switch -CaseSensitive ($t.event.type){
        'PARTY_JOINED' { Assert-Keys $t.event @('type') 'Legacy guide start trigger' }
        'GUIDE_STARTED' { Assert-Keys $t.event @('type') 'Guide start trigger'; Assert-True ($cats[$t.categoryId].worldId -ceq 'BERN') 'Guide start requires Bern' }
        'RAID_RETURNED' { Assert-Keys $t.event @('type','raidWorldId') 'Raid return trigger'; Assert-True ($cats[$t.categoryId].worldId -ceq 'BERN' -and $t.event.raidWorldId -cin @('VALTAN_ARENA','KAKULSAYDON_ARENA')) 'Raid return requires Bern and a supported source raid' }
        'WORLD_RETURNED' { Assert-Keys $t.event @('type','sourceWorldId') 'World return trigger'; Assert-True ($cats[$t.categoryId].worldId -ceq 'BERN' -and $t.event.sourceWorldId -cin @('MAHARAKA','COLOSSEUM')) 'World return requires Bern and a supported source world' }
        'SPACE_ENTER' { Assert-Keys $t.event @('type','boxId','position','yawDegrees','halfExtents','anchorPlacementId') 'Box trigger'; Assert-Id $t.event.boxId 'Box'; Assert-Vector $t.event.position -100000 'Box position'; Assert-Vector $t.event.halfExtents 0.01 'Box half extents'; Assert-Number $t.event.yawDegrees -36000 36000 'Box yaw'; if($t.event.anchorPlacementId -cne ''){ Assert-True (@($worlds[$t.categoryId].placements | Where-Object placementId -CEQ $t.event.anchorPlacementId).Count -eq 1) 'Box anchor is not a current world placement' } }
        'BOSS_PATTERN_STARTED' { Assert-Keys $t.event @('type','patternId') 'Pattern trigger'; Assert-Id $t.event.patternId 'Pattern'; Assert-True ($bootstrap -cmatch ('(?m)^PATTERN\t[^\t]+\t'+[Regex]::Escape($t.event.patternId)+'\t')) 'Pattern is not published' }
        'HELP_COMMAND' { Assert-Keys $t.event @('type','commandId') 'Help trigger'; Assert-True ($commandIds.ContainsKey($t.event.commandId)) 'Help command is missing' }
        default { throw 'Unknown Guide event type' }
    } }
    $placement=Clone $b.placement; foreach($k in @('schema','formatVersion','revision','guideId')){ $placement.PSObject.Properties.Remove($k) }
    foreach($k in @('schema','formatVersion','revision','guideId')){ $combat.PSObject.Properties.Remove($k) }
    $runtime=[ordered]@{schema='lostark.guide-runtime';formatVersion=1;revision=0;guideId=$g.guideId;characterClass=$g.characterClass;displayName=$g.displayName;categories=$b.catalog.categories;placement=$placement;prompts=$b.prompts.prompts;triggers=$b.triggers.triggers;combat=$combat}
    $sha=[Security.Cryptography.SHA256]::Create(); try{$digest=[BitConverter]::ToString($sha.ComputeHash($utf8.GetBytes((Json $runtime)))).Replace('-','')}finally{$sha.Dispose()}; $runtime.revision=[Convert]::ToUInt64($digest.Substring(0,12),16)
    [pscustomobject]$runtime
}
# Only fields changed by this editor are rebased. Stable IDs make independent row edits commute.
function Merge-Value($before,$draft,$current,[string]$where) {
    if((Json $before) -ceq (Json $draft)){ return ,$current }
    if((Json $before) -ceq (Json $current) -or (Json $draft) -ceq (Json $current)){ return ,$draft }
    if($before -is [pscustomobject] -and $draft -is [pscustomobject] -and $current -is [pscustomobject]){ $merged=Clone $current; foreach($property in $draft.PSObject.Properties){ $name=$property.Name; if($name -eq 'revision'){continue}; $old=$before.PSObject.Properties[$name]; $now=$current.PSObject.Properties[$name]; $oldValue=$null; $nowValue=$null; if($old){$oldValue=$old.Value}; if($now){$nowValue=$now.Value}; $value=Merge-Value $oldValue $property.Value $nowValue "$where/$name"; $merged | Add-Member -NotePropertyName $name -NotePropertyValue $value -Force }; return $merged }
    $idKey=switch -Regex ($where){ '/categories$' {'categoryId'} '/guides$' {'guideId'} '/prompts$' {'promptId'} '/triggers$' {'triggerId'} '/combos$' {'comboId'} '/commands$' {'commandId'} default {''} }
    if($idKey -and $before -is [array] -and $draft -is [array] -and $current -is [array]){ $rows=[Collections.Generic.List[object]]::new(); foreach($row in $current){$rows.Add($row)}; $ids=@(@($before)+@($draft) | ForEach-Object { $_.$idKey } | Select-Object -Unique); foreach($id in $ids){ $old=@($before|Where-Object {$_.$idKey -ceq $id}); $next=@($draft|Where-Object {$_.$idKey -ceq $id}); $now=@($current|Where-Object {$_.$idKey -ceq $id}); $oldValue=if($old.Count){$old[0]}else{$null}; $nextValue=if($next.Count){$next[0]}else{$null}; $nowValue=if($now.Count){$now[0]}else{$null}; $value=Merge-Value $oldValue $nextValue $nowValue "$where/$id"; for($i=$rows.Count-1;$i -ge 0;$i--){if($rows[$i].$idKey -ceq $id){$rows.RemoveAt($i)}}; if($null -ne $value){$rows.Add($value)} }; return ,$rows.ToArray() }
    throw "Guide Save conflict at $where. Current disk and editor draft preserved."
}
function Write-Transaction([Collections.IDictionary]$outputs) {
    $locks=[Collections.Generic.List[Threading.Mutex]]::new()
    $staged=@{}; $backups=@{}; $old=@{}; $expected=@{}; $installed=@{}; $retainBackup=@{}
    $installedOrder=[Collections.Generic.List[string]]::new(); $token=[Guid]::NewGuid().ToString('N')
    try {
        foreach($path in @($outputs.Keys|Sort-Object)){ Assert-RepositoryPath $path; $locks.Add((Enter-PublishDestinationMutex (Get-PublishDestinationMutexName $path))) }
        foreach($path in $outputs.Keys){
            [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($path))|Out-Null
            $old[$path]=if([IO.File]::Exists($path)){Get-PublishFileSha256 $path}else{''}
            $staged[$path]="$path.staging.$token"; $backups[$path]="$path.rollback.$token"
            [IO.File]::WriteAllText($staged[$path],$outputs[$path],$utf8)
            $expected[$path]=Get-PublishFileSha256 $staged[$path]
        }
        Assert-PublishSourceSnapshots $sources
        foreach($path in $outputs.Keys){
            Assert-PublishSourceSnapshots $sources
            if($old[$path]){
                Assert-True (Test-PublishFileHash $path $old[$path] 'Guide') "Destination changed: $path"
                [IO.File]::Replace($staged[$path],$path,$backups[$path])
            }else{ [IO.File]::Move($staged[$path],$path) }
            # Register a successful replacement before any subsequent operation can fail.
            $installed[$path]=$expected[$path]; $installedOrder.Add($path)
            if($old[$path]){ Assert-True ((Get-PublishFileSha256 $backups[$path]) -ceq $old[$path]) "Concurrent replacement: $path" }
            Assert-True (Test-PublishFileHash $path $expected[$path] 'Guide installed') "Destination changed after install: $path"
            if($sources.ContainsKey($path)){ $sources[$path]=[Convert]::ToBase64String($utf8.GetBytes($outputs[$path])) }
        }
    } catch {
        for($i=$installedOrder.Count-1;$i -ge 0;$i--){
            $path=$installedOrder[$i]
            try {
                if(Test-PublishFileHash $path $installed[$path] 'Guide rollback'){
                    if([IO.File]::Exists($backups[$path])){[IO.File]::Replace($backups[$path],$path,[NullString]::Value)}else{[IO.File]::Delete($path)}
                }else{
                    $retainBackup[$path]=$true
                    Write-Warning "Guide rollback preserved a concurrent destination edit and retained backup: $($backups[$path])"
                }
            }catch{
                $retainBackup[$path]=$true
                Write-Warning "Guide rollback could not restore $path. Backup retained: $($backups[$path]). $($_.Exception.Message)"
            }
        }
        throw
    } finally {
        foreach($path in $staged.Values){if([IO.File]::Exists($path)){try{[IO.File]::Delete($path)}catch{Write-Warning "Guide stage cleanup failed: $path"}}}
        foreach($path in $backups.Keys){if(-not $retainBackup.ContainsKey($path) -and [IO.File]::Exists($backups[$path])){try{[IO.File]::Delete($backups[$path])}catch{Write-Warning "Guide backup cleanup failed: $($backups[$path])"}}}
        foreach($mutex in $locks){Close-PublishDestinationMutex $mutex 'Guide'}
    }
}
try {
    $bundle=Read-Bundle
    if($Mode -eq 'Save'){
        Assert-True ($BaselinePath -and $DraftPath) 'Save requires baseline and draft files'
        $baseline=Read-Json $BaselinePath $false; $draft=Read-Json $DraftPath $false
        $null=Make-Runtime $draft
        $merged=Merge-Value $baseline $draft $bundle 'Guide'
        foreach($key in $files.Keys){if((Json $bundle.$key) -cne (Json $merged.$key)){ $merged.$key.revision=[uint64]$bundle.$key.revision+1 }}
        $null=Make-Runtime $merged; $outputs=[ordered]@{}
        foreach($key in $files.Keys){if((Json $bundle.$key) -cne (Json $merged.$key)){ $outputs[(Join-Path $RepositoryRoot ('Data/Guide/'+$files[$key]))]=(ConvertTo-Json -InputObject $merged.$key -Depth 80)+"`n" }}
        if($outputs.Count){Write-Transaction $outputs}; Write-Output 'Guide Save succeeded. Source only; use Publish to replace runtime data.'
    } else {
        $runtime=Make-Runtime $bundle; Assert-PublishSourceSnapshots $sources
        $text=(ConvertTo-Json -InputObject $runtime -Depth 80)+"`n"; $outputs=[ordered]@{}
        foreach($side in @('Server','Client')){ $path=Join-Path $RepositoryRoot "$side/Bin/DataFiles/Guide/Guide.runtime.json"; $outputs[$path]=$text; if($Mode -eq 'CheckPublished'){Assert-True ([IO.File]::Exists($path) -and [IO.File]::ReadAllText($path,$utf8).Replace("`r`n","`n") -ceq $text.Replace("`r`n","`n")) "Guide runtime is stale: $path"} }
        if($Mode -eq 'Publish'){Write-Transaction $outputs}
        Write-Output "Guide $Mode succeeded. revision=$($runtime.revision); prompts=$(@($runtime.prompts).Count); triggers=$(@($runtime.triggers).Count); combos=$(@($runtime.combat.combos).Count)"
    }
} catch { Write-Error $_; exit 1 }
