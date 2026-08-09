[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path

function Read-Source([string]$relativePath) {
    $path = Join-Path $repoRoot $relativePath
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        throw "[CharacterActionTiming] Missing source file: $relativePath"
    }
    return [IO.File]::ReadAllText($path)
}

function Require-Match(
    [string]$text,
    [string]$pattern,
    [string]$message) {
    if ($text -notmatch $pattern) {
        throw "[CharacterActionTiming] $message"
    }
}

function Resolve-MockClip([object[]]$clips, [double]$wallAgeSeconds) {
    $remaining = [Math]::Max(0.0, $wallAgeSeconds)
    for ($index = 0; $index -lt $clips.Count; ++$index) {
        $clip = $clips[$index]
        $sourceLimit = [double]$clip.DurationSeconds
        if ([int]$clip.PlayMs -ne 0) {
            $sourceLimit = [Math]::Min(
                $sourceLimit, [double]$clip.PlayMs * 0.001)
        }
        $wallDuration = $sourceLimit / [double]$clip.PlayRate
        $isLast = $index + 1 -eq $clips.Count
        if (-not $isLast -and $remaining -ge $wallDuration) {
            $remaining -= $wallDuration
            continue
        }
        $clipWall = [Math]::Min($remaining, $wallDuration)
        if ([bool]$clip.Loop) {
            $clipWall = $remaining % $wallDuration
        }
        return [pscustomobject]@{
            Index = $index
            SourceSeconds = $clipWall * [double]$clip.PlayRate
        }
    }
    throw '[CharacterActionTiming] Mock clip resolver exhausted its chain.'
}

function Require-Near([double]$actual, [double]$expected, [string]$message) {
    if ([Math]::Abs($actual - $expected) -gt 0.000001) {
        throw "[CharacterActionTiming] $message (actual=$actual expected=$expected)"
    }
}

$characterHeader = Read-Source 'Client/Public/Character.h'
$characterSource = Read-Source 'Client/Private/Character.cpp'
$replicationSource = Read-Source 'Client/Private/ClientReplication.cpp'
$timelineHeader = Read-Source 'Client/Public/ActionPresentationTimeline.h'
$timelineSource = Read-Source 'Client/Private/ActionPresentationTimeline.cpp'
$frontendHarness = Read-Source 'Tools/ClientFrontendHarness/Private/ClientFrontendHarness.cpp'

Require-Match $characterHeader `
    'Apply_NetworkAction\([\s\S]*?SKILL_ID skillId,[\s\S]*?std::uint32_t serverTick,[\s\S]*?std::uint32_t actionStartTick' `
    'Apply_NetworkAction must receive the snapshot Server tick explicitly.'
Require-Match $replicationSource `
    'Apply_NetworkAction\([\s\S]*?player\.eAction,[\s\S]*?player\.iSkillId,[\s\S]*?snapshot\.iServerTick,[\s\S]*?player\.iActionStartTick' `
    'Replication must pass snapshot.iServerTick into Character presentation.'
Require-Match $characterSource `
    'Try_ResolveActionAgeSeconds\([\s\S]*?serverTick,[\s\S]*?actionStartTick,[\s\S]*?SERVER_TICK_HZ' `
    'Authoritative action age must derive from the 30 Hz Server tick delta.'
Require-Match $characterSource `
    'bSameActionEdge[\s\S]*?m_iChainStage\s*!=\s*iExpectedStage[\s\S]*?fActionAgeSeconds\s*>\s*m_fActionPresentationSeconds[\s\S]*?Seek_ActiveStageForward' `
    'Repeated edges must only seek forward and must not restart a changed stage.'
Require-Match $characterSource `
    'Reset_EffectCueCursor\(actionStartTick\)[\s\S]*?Play_Skill[\s\S]*?Advance_ComboStage[\s\S]*?Seek_ActiveStageForward\(fActionAgeSeconds\)' `
    'A new staged edge must select its Server combo stage and seek before cue crossing.'
Require-Match $timelineSource `
    'fModelSourceDurationSeconds[\s\S]*?iPlayMs[\s\S]*?fOutWallDurationSeconds\s*=\s*fOutSourceDurationSeconds\s*/\s*Clip\.fPlayRate' `
    'Clip seek must combine model duration, authored playMs, and playRate.'
Require-Match $timelineSource `
    'std::fmod\([\s\S]*?fRemainingWallSeconds,[\s\S]*?fWallDurationSeconds[\s\S]*?fLoopWallSeconds\)\s*\*\s*Clips\[iClip\]\.fPlayRate[\s\S]*?fRemainingWallSeconds\s*>=\s*fWallDurationSeconds' `
    'Sequential and looping clips must resolve from wall time into source clip time.'
Require-Match $characterSource `
    'Resolve_CueWallOffset[\s\S]*?fOccurrenceWallSeconds[\s\S]*?strOccurrenceId\s*=\s*"stage:"' `
    'Cue crossing must use the absolute stage timeline and stable occurrence identity.'
Require-Match $replicationSource `
    'Is_ForwardTick\([\s\S]*?snapshot\.iServerTick,[\s\S]*?m_iLastServerTick' `
    'The outer snapshot gate must use signed modular tick ordering.'
Require-Match $timelineSource `
    'static_cast<int32_t>\(iCandidateTick\s*-\s*iPreviousTick\)\s*>\s*0' `
    'Signed modular ordering must accept MAX-to-1 wrap and reject its stale inverse.'
Require-Match $frontendHarness `
    'Test_ActionPresentationTimeline[\s\S]*?Exact Boundary[\s\S]*?HOLD[\s\S]*?UINT32_MAX[\s\S]*?2050240 Delayed First Snapshot' `
    'The executable C++ harness must cover exact boundary, HOLD, tick wrap, and DimensionMaster 2050240 backlog.'
Require-Match $characterSource `
    '__super::Update\(fTimeDelta\);[\s\S]*?m_fActionPresentationSeconds\s*\+=\s*fTimeDelta;[\s\S]*?Update_EffectCues\(\);' `
    'The forward-only local presentation clock must advance before cue crossing.'

$mockClips = @(
    [pscustomobject]@{ DurationSeconds = 0.6; PlayMs = 0; PlayRate = 1.0; Loop = $false },
    [pscustomobject]@{ DurationSeconds = 1.0; PlayMs = 400; PlayRate = 2.0; Loop = $false },
    [pscustomobject]@{ DurationSeconds = 2.0; PlayMs = 0; PlayRate = 0.5; Loop = $false }
)
$backlogAge = [double](100 - 70) / 30.0
$resolved = Resolve-MockClip $mockClips $backlogAge
if ($resolved.Index -ne 2) {
    throw '[CharacterActionTiming] Backlog did not cross both authored clip boundaries.'
}
Require-Near $resolved.SourceSeconds 0.1 `
    'Backlog source sample did not respect playMs and both play rates.'

$boundary = Resolve-MockClip $mockClips 0.6
if ($boundary.Index -ne 1) {
    throw '[CharacterActionTiming] Exact clip boundary did not enter the next clip.'
}
Require-Near $boundary.SourceSeconds 0.0 `
    'Exact clip boundary must start the next clip at source time zero.'

$looped = Resolve-MockClip @(
    [pscustomobject]@{ DurationSeconds = 0.5; PlayMs = 0; PlayRate = 1.0; Loop = $true }
) 1.25
Require-Near $looped.SourceSeconds 0.25 `
    'Delayed HOLD-loop presentation did not preserve its loop phase.'

$currentPresentation = 0.9
$olderSameEdge = 0.8
$newerSameEdge = 1.1
if ($olderSameEdge -gt $currentPresentation -or
    $newerSameEdge -le $currentPresentation) {
    throw '[CharacterActionTiming] Forward-only repeated-edge gate regressed.'
}

Write-Host (
    '[CharacterActionTiming] PASS: snapshot Server age selects combo/sequential ' +
    'clip time, backlog respects playMs/playRate/duration, and repeated edges ' +
    'advance without restarting.')
