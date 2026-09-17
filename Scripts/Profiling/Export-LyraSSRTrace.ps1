param(
    [Parameter(Mandatory = $true)]
    [string]$TraceFile,

    [string]$OutputDirectory = "",

    [string]$UnrealInsightsPath = "C:\Program Files\Epic Games\UE_5.5\Engine\Binaries\Win64\UnrealInsights.exe"
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

function Get-NearestRankValue {
    param(
        [double[]]$SortedValues,
        [double]$Percentile
    )

    if ($SortedValues.Count -eq 0) { return 0.0 }
    $index = [Math]::Max(0, [Math]::Min($SortedValues.Count - 1, [Math]::Ceiling($Percentile * $SortedValues.Count) - 1))
    return $SortedValues[$index]
}

function Get-Distribution {
    param([double[]]$Values)

    if ($Values.Count -eq 0) {
        return [pscustomobject]@{
            Count = 0; Average = 0.0; P50 = 0.0; P95 = 0.0; P99 = 0.0
            Minimum = 0.0; Maximum = 0.0; Total = 0.0
        }
    }

    [double[]]$sorted = @($Values | Sort-Object)
    $measure = $sorted | Measure-Object -Average -Minimum -Maximum -Sum
    return [pscustomobject]@{
        Count = $sorted.Count
        Average = [double]$measure.Average
        P50 = Get-NearestRankValue $sorted 0.50
        P95 = Get-NearestRankValue $sorted 0.95
        P99 = Get-NearestRankValue $sorted 0.99
        Minimum = [double]$measure.Minimum
        Maximum = [double]$measure.Maximum
        Total = [double]$measure.Sum
    }
}

function Read-CounterValues {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) { return [double[]]@() }
    return [double[]]@(
        Import-Csv -LiteralPath $Path |
            ForEach-Object { [double]::Parse($_.Value, [Globalization.CultureInfo]::InvariantCulture) }
    )
}

function Get-FinalCounterValue {
    param([string]$Path)

    [double[]]$values = Read-CounterValues $Path
    if ($values.Count -eq 0) { return [double]::NaN }
    return $values[-1]
}

function Convert-ToIntOrDefault {
    param(
        [double]$Value,
        [int]$DefaultValue = -1
    )

    if ([double]::IsNaN($Value) -or [double]::IsInfinity($Value)) { return $DefaultValue }
    return [int]$Value
}

function Get-MappedName {
    param(
        [double]$Value,
        [hashtable]$Names,
        [string]$MissingName = 'Missing'
    )

    $id = Convert-ToIntOrDefault $Value
    if ($id -lt 0 -or -not $Names.ContainsKey($id)) { return $MissingName }
    return $Names[$id]
}

function Get-InvalidReasonNames {
    param([double]$MaskValue)

    if ([double]::IsNaN($MaskValue) -or [double]::IsInfinity($MaskValue)) { return 'Missing' }
    [uint32]$mask = $MaskValue
    if ($mask -eq 0) { return '' }

    $reasonBits = [ordered]@{
        0x000001 = 'NotDedicatedServer'
        0x000002 = 'CharacterCount'
        0x000004 = 'ActiveSnapshotSelection'
        0x000008 = 'ParallelCVars'
        0x000010 = 'SnapshotCallsPerFrame'
        0x000020 = 'UnexpectedRewindHit'
        0x000040 = 'SnapshotRange'
        0x000080 = 'SingleFrameFallback'
        0x000100 = 'PoseReconstruction'
        0x000200 = 'StateRestore'
        0x000400 = 'TopologyMismatch'
        0x000800 = 'MissedInjection'
        0x001000 = 'AnimationStateMutation'
        0x002000 = 'Arguments'
        0x004000 = 'Aborted'
        0x008000 = 'MeshSettings'
        0x010000 = 'RewindRequestCount'
        0x020000 = 'DebugDraw'
        0x040000 = 'SnapshotReadiness'
        0x080000 = 'AnimationExecution'
        0x100000 = 'MaxTickRate'
        0x200000 = 'NoSnapshotSamples'
        0x400000 = 'NoServerFrames'
    }
    $names = [System.Collections.Generic.List[string]]::new()
    foreach ($entry in $reasonBits.GetEnumerator()) {
        if (($mask -band [uint32]$entry.Key) -ne 0) { $names.Add([string]$entry.Value) }
    }
    return $names -join '|'
}

function Add-TimingMetric {
    param(
        [System.Collections.Generic.List[object]]$Rows,
        [object[]]$Events,
        [string]$Category,
        [string]$Metric,
        [string]$TimerPattern,
        [int]$ServerFrameCount,
        [string]$ThreadDomain,
        [string]$Notes,
        [ValidateSet('Any', 'GameThread', 'NonGameThread')]
        [string]$EventThreadDomain = 'Any'
    )

    [double[]]$microseconds = @(
        $Events |
            Where-Object {
                $event = $_
                $timerMatches = $event.TimerName -like $TimerPattern
                $threadMatches = switch ($EventThreadDomain) {
                    'GameThread' { $event.ThreadName -match '^GameThread' }
                    'NonGameThread' { $event.ThreadName -notmatch '^GameThread' }
                    default { $true }
                }
                $timerMatches -and $threadMatches
            } |
            ForEach-Object { 1000000.0 * [double]::Parse($_.Duration, [Globalization.CultureInfo]::InvariantCulture) }
    )
    $stats = Get-Distribution $microseconds
    $Rows.Add([pscustomobject]@{
        Category = $Category
        Metric = $Metric
        ThreadDomain = $ThreadDomain
        Unit = 'us'
        Count = $stats.Count
        Average = $stats.Average
        P50 = $stats.P50
        P95 = $stats.P95
        P99 = $stats.P99
        Minimum = $stats.Minimum
        Maximum = $stats.Maximum
        Total = $stats.Total
        AverageTotalPerServerFrame = if ($ServerFrameCount -gt 0) { $stats.Total / $ServerFrameCount } else { 0.0 }
        Notes = $Notes
    }) | Out-Null
}

$resolvedTrace = (Resolve-Path -LiteralPath $TraceFile).Path
if (-not (Test-Path -LiteralPath $UnrealInsightsPath -PathType Leaf)) {
    throw "UnrealInsights.exe does not exist: $UnrealInsightsPath"
}

$traceBaseName = [IO.Path]::GetFileNameWithoutExtension($resolvedTrace)
if ([string]::IsNullOrWhiteSpace($OutputDirectory)) {
    $OutputDirectory = Join-Path (Split-Path -Parent $resolvedTrace) ($traceBaseName + '_analysis')
}
$null = New-Item -ItemType Directory -Path $OutputDirectory -Force
$resolvedOutput = (Resolve-Path -LiteralPath $OutputDirectory).Path

$eventsPath = Join-Path $resolvedOutput 'timing_events.csv'
$logPath = Join-Path $resolvedOutput 'unreal_insights_export.log'
$responsePath = Join-Path $resolvedOutput 'export_commands.rsp'
$summaryPath = Join-Path $resolvedOutput 'summary.csv'
$metadataPath = Join-Path $resolvedOutput 'metadata.csv'

$counterFiles = [ordered]@{
    SessionValid = 'session_valid.csv'
    InvalidReasonMask = 'invalid_reason_mask.csv'
    Variant = 'variant.csv'
    Profile = 'profile.csv'
    RewindScenario = 'rewind_scenario.csv'
    DedicatedServer = 'dedicated_server.csv'
    CharacterCount = 'character_count.csv'
    ConfiguredNetServerMaxTickRate = 'configured_net_server_max_tick_rate.csv'
    EffectiveEngineMaxTickRate = 'effective_engine_max_tick_rate.csv'
    ActualMeasurementSeconds = 'actual_measurement_seconds.csv'
    FrameMilliseconds = 'frame_milliseconds.csv'
    ServerFrames = 'server_frames.csv'
    RewindRequests = 'rewind_requests.csv'
    RewindHits = 'rewind_hits.csv'
    RewindMisses = 'rewind_misses.csv'
    RewindInvalidResults = 'rewind_invalid_results.csv'
    MissedInjections = 'missed_injections.csv'
    ParallelCapableAnimInstances = 'parallel_capable_anim_instances.csv'
    GameThreadOnlyAnimInstances = 'game_thread_only_anim_instances.csv'
}

$generatedPaths = @($eventsPath, $logPath, $responsePath, $summaryPath, $metadataPath)
$generatedPaths += @($counterFiles.Values | ForEach-Object { Join-Path $resolvedOutput $_ })
foreach ($generatedPath in $generatedPaths) {
    if (Test-Path -LiteralPath $generatedPath -PathType Leaf) {
        Remove-Item -LiteralPath $generatedPath -Force
    }
}

$timerNames = @(
    'LyraProfile_MeshTickGT',
    'LyraProfile_SnapshotUpdateGT',
    'LyraProfile_RewindTotal',
    'LyraProfile_Rewind_SnapshotLookup',
    'LyraProfile_Rewind_PoseReconstruction',
    'LyraProfile_Rewind_PhysicsCollision',
    'FParallelAnimationEvaluationTask',
    'FParallelAnimationCompletionTask',
    'CharacterMesh0',
    'USkeletalMeshComponent_CompleteParallelAnimationEvaluation',
    'USkeletalMeshComponent::BlockOnParallelEvaluationTask',
    'PerformAnimEvaluation'
) -join ','

$quote = [char]34
$commands = [System.Collections.Generic.List[string]]::new()
$eventsInsightsPath = $eventsPath.Replace('\', '/')
$commands.Add("TimingInsights.ExportTimingEvents $quote$eventsInsightsPath$quote -columns=ThreadId,ThreadName,TimerId,TimerName,StartTime,EndTime,Duration,Depth -timers=$quote$timerNames$quote -region=LyraSSR_Measurement")
foreach ($entry in $counterFiles.GetEnumerator()) {
    $counterPath = Join-Path $resolvedOutput $entry.Value
    $counterInsightsPath = $counterPath.Replace('\', '/')
    $commands.Add("TimingInsights.ExportCounterValues $quote$counterInsightsPath$quote -counter=$quote" + "LyraSSR/$($entry.Key)" + "$quote -region=LyraSSR_Measurement")
}
$commands | Set-Content -LiteralPath $responsePath -Encoding Unicode

$insightsArguments = @(
    "-OpenTraceFile=$resolvedTrace",
    "-ABSLOG=$logPath",
    '-AutoQuit',
    '-NoUI',
    "-ExecOnAnalysisCompleteCmd=@=$responsePath",
    '-log'
)
$insightsProcess = Start-Process -FilePath $UnrealInsightsPath -ArgumentList $insightsArguments -Wait -PassThru -WindowStyle Hidden
if ($insightsProcess.ExitCode -ne 0) {
    throw "Unreal Insights export failed with exit code $($insightsProcess.ExitCode). See $logPath"
}
if (-not (Test-Path -LiteralPath $eventsPath)) {
    throw "Timing event export was not created. Verify cpu,frame,task,counters,bookmark,region trace channels. See $logPath"
}

$events = @(Import-Csv -LiteralPath $eventsPath)
function Counter-Path([string]$Name) { Join-Path $resolvedOutput $counterFiles[$Name] }

$snapshotEvents = @($events | Where-Object TimerName -eq 'LyraProfile_SnapshotUpdateGT')
$meshEvents = @($events | Where-Object TimerName -eq 'LyraProfile_MeshTickGT')
$workerEvents = @($events | Where-Object {
    $_.TimerName -eq 'CharacterMesh0' -and $_.ThreadName -notmatch '^GameThread'
})
$snapshotTimerCount = $snapshotEvents.Count
$parallelAnimationTickCount = $workerEvents.Count
$nonParallelAnimationTickCount = [Math]::Max(0, $meshEvents.Count - $parallelAnimationTickCount)

$serverFrameCounter = Get-FinalCounterValue (Counter-Path 'ServerFrames')
$serverFrameCount = if ([double]::IsNaN($serverFrameCounter)) { 0 } else { [int]$serverFrameCounter }
[double[]]$frameMilliseconds = Read-CounterValues (Counter-Path 'FrameMilliseconds')
$frameStats = Get-Distribution $frameMilliseconds
[double[]]$frameHertz = @($frameMilliseconds | Where-Object { $_ -gt 0.0 } | ForEach-Object { 1000.0 / $_ })
$hertzStats = Get-Distribution $frameHertz

$rows = [System.Collections.Generic.List[object]]::new()
Add-TimingMetric $rows $events 'Snapshot' 'SnapshotSaveGT' 'LyraProfile_SnapshotUpdateGT' $serverFrameCount 'GameThread' 'Snapshot serialization and history update only.'
Add-TimingMetric $rows $events 'Rewind' 'ServerSideRewindRequestGT' 'LyraProfile_RewindTotal' $serverFrameCount 'GameThread' 'Deterministic interpolated-miss request from server function entry through final decision; excludes network round trip.'
Add-TimingMetric $rows $events 'MeshTick' 'MeshTickGT' 'LyraProfile_MeshTickGT' $serverFrameCount 'GameThread' 'Game-thread mesh tick wall time. Do not add nested completion/wait rows again.'
Add-TimingMetric $rows $events 'MeshTick' 'ParallelAnimationWorkerCPU' 'CharacterMesh0' $serverFrameCount 'WorkerThreads' 'Non-game-thread UObject context scope around ParallelAnimationEvaluation. Sum is server-wide worker CPU work, not wall-clock frame latency.' 'NonGameThread'
Add-TimingMetric $rows $events 'MeshTick' 'ParallelAnimationCompletionGT' '*FParallelAnimationCompletionTask*' $serverFrameCount 'GameThread' 'Game-thread task that accepts worker results; may overlap the MeshTickGT accounting hierarchy.'
Add-TimingMetric $rows $events 'MeshTick' 'ParallelAnimationFinalizeGT' 'USkeletalMeshComponent_CompleteParallelAnimationEvaluation' $serverFrameCount 'GameThread' 'Result swap and PostAnimEvaluation inside the completion task. Do not add both inclusive completion rows.'
Add-TimingMetric $rows $events 'MeshTick' 'ParallelAnimationBlockWaitGT' '*BlockOnParallelEvaluationTask*' $serverFrameCount 'GameThread' 'Blocking latency on the game thread; retain in frame latency but do not count as active CPU work.'
$rows.Add([pscustomobject]@{
    Category = 'ServerFrame'
    Metric = 'ServerFrameTime'
    ThreadDomain = 'ServerWallClock'
    Unit = 'ms'
    Count = $frameStats.Count
    Average = $frameStats.Average
    P50 = $frameStats.P50
    P95 = $frameStats.P95
    P99 = $frameStats.P99
    Minimum = $frameStats.Minimum
    Maximum = $frameStats.Maximum
    Total = $frameStats.Total
    AverageTotalPerServerFrame = $frameStats.Average
    Notes = 'Dedicated-server wall-clock interval recorded once per measured frame.'
}) | Out-Null
$rows.Add([pscustomobject]@{
    Category = 'ServerFrame'
    Metric = 'EffectiveServerHz'
    ThreadDomain = 'ServerWallClock'
    Unit = 'Hz'
    Count = $hertzStats.Count
    Average = if ($frameStats.Total -gt 0.0) { 1000.0 * $frameStats.Count / $frameStats.Total } else { 0.0 }
    P50 = $hertzStats.P50
    P95 = $hertzStats.P95
    P99 = $hertzStats.P99
    Minimum = $hertzStats.Minimum
    Maximum = $hertzStats.Maximum
    Total = $hertzStats.Total
    AverageTotalPerServerFrame = 0.0
    Notes = 'Average is frame count divided by elapsed frame intervals; percentiles/min/max are per-frame instantaneous Hz.'
}) | Out-Null
$rows | Export-Csv -LiteralPath $summaryPath -NoTypeInformation -Encoding UTF8

$variantId = Get-FinalCounterValue (Counter-Path 'Variant')
$profileId = Get-FinalCounterValue (Counter-Path 'Profile')
$rewindScenarioId = Get-FinalCounterValue (Counter-Path 'RewindScenario')
$invalidReasonMask = Get-FinalCounterValue (Counter-Path 'InvalidReasonMask')
$variantNames = @{ 0 = 'SkeletalMesh'; 1 = 'AnimNode' }
$profileNames = @{ 0 = 'None'; 1 = 'SteadyTrace'; 2 = 'Snapshot'; 3 = 'RewindMicro'; 4 = 'RewindLoad' }
$rewindScenarioNames = @{ 0 = 'None'; 1 = 'InterpolatedMiss' }
$metadata = [ordered]@{
    TraceFile = $resolvedTrace
    AnalysisValid = $true
    AnalysisInvalidReasons = ''
    SessionValid = Get-FinalCounterValue (Counter-Path 'SessionValid')
    InvalidReasonMask = $invalidReasonMask
    InvalidReasonNames = Get-InvalidReasonNames $invalidReasonMask
    Variant = Get-MappedName $variantId $variantNames
    Profile = Get-MappedName $profileId $profileNames
    RewindScenario = Get-MappedName $rewindScenarioId $rewindScenarioNames
    DedicatedServer = Get-FinalCounterValue (Counter-Path 'DedicatedServer')
    CharacterCount = Get-FinalCounterValue (Counter-Path 'CharacterCount')
    ConfiguredNetServerMaxTickRate = Get-FinalCounterValue (Counter-Path 'ConfiguredNetServerMaxTickRate')
    EffectiveEngineMaxTickRate = Get-FinalCounterValue (Counter-Path 'EffectiveEngineMaxTickRate')
    ActualMeasurementSeconds = Get-FinalCounterValue (Counter-Path 'ActualMeasurementSeconds')
    ServerFrames = $serverFrameCount
    SnapshotCalls = $snapshotTimerCount
    RewindRequests = Get-FinalCounterValue (Counter-Path 'RewindRequests')
    RewindHits = Get-FinalCounterValue (Counter-Path 'RewindHits')
    RewindMisses = Get-FinalCounterValue (Counter-Path 'RewindMisses')
    RewindInvalidResults = Get-FinalCounterValue (Counter-Path 'RewindInvalidResults')
    MissedInjections = Get-FinalCounterValue (Counter-Path 'MissedInjections')
    ParallelAnimationMeshTicks = $parallelAnimationTickCount
    NonParallelAnimationMeshTicks = $nonParallelAnimationTickCount
    ParallelCapableAnimInstances = Get-FinalCounterValue (Counter-Path 'ParallelCapableAnimInstances')
    GameThreadOnlyAnimInstances = Get-FinalCounterValue (Counter-Path 'GameThreadOnlyAnimInstances')
}

$invalidReasons = [System.Collections.Generic.List[string]]::new()
foreach ($entry in $counterFiles.GetEnumerator()) {
    if (@(Read-CounterValues (Counter-Path $entry.Key)).Count -eq 0) {
        $invalidReasons.Add("missing counter: LyraSSR/$($entry.Key)")
    }
}
if ($metadata.SessionValid -ne 1) { $invalidReasons.Add("SessionValid is not 1") }
if ($metadata.InvalidReasonMask -ne 0) { $invalidReasons.Add("InvalidReasonMask is $($metadata.InvalidReasonMask)") }
if ($metadata.DedicatedServer -ne 1) { $invalidReasons.Add("capture is not from NM_DedicatedServer") }
if ($metadata.CharacterCount -ne 10) { $invalidReasons.Add("CharacterCount is $($metadata.CharacterCount), expected 10") }
if ($metadata.EffectiveEngineMaxTickRate -ne 0) { $invalidReasons.Add("EffectiveEngineMaxTickRate is $($metadata.EffectiveEngineMaxTickRate), expected uncapped 0") }
if ($metadata.SnapshotCalls -le 0) { $invalidReasons.Add("no snapshot timing events were exported") }
if ($metadata.ParallelAnimationMeshTicks -le 0) { $invalidReasons.Add("no native parallel animation mesh tick was observed") }
if ($metadata.NonParallelAnimationMeshTicks -ne 0) { $invalidReasons.Add("non-parallel animation mesh ticks were observed") }
if ($metadata.ParallelCapableAnimInstances -le 0) { $invalidReasons.Add("no parallel-capable animation instance was observed") }
if ($metadata.GameThreadOnlyAnimInstances -ne 0) { $invalidReasons.Add("game-thread-only animation instances were observed") }
if ($metadata.RewindInvalidResults -ne 0) { $invalidReasons.Add("invalid rewind results were observed") }
if ($metadata.MissedInjections -ne 0) { $invalidReasons.Add("rewind load injections were missed") }
if ($frameStats.Count -ne $serverFrameCount) { $invalidReasons.Add("frame counter samples ($($frameStats.Count)) do not match ServerFrames ($serverFrameCount)") }
if ($hertzStats.Count -ne $serverFrameCount) { $invalidReasons.Add("non-positive server frame intervals were observed") }

$rewindRequestCount = Convert-ToIntOrDefault $metadata.RewindRequests
$rewindHitCount = Convert-ToIntOrDefault $metadata.RewindHits
$rewindMissCount = Convert-ToIntOrDefault $metadata.RewindMisses
if ($rewindHitCount + $rewindMissCount -ne $rewindRequestCount) {
    $invalidReasons.Add("rewind hit/miss total ($($rewindHitCount + $rewindMissCount)) does not match requests ($rewindRequestCount)")
}
switch ($metadata.Profile) {
    'Snapshot' {
        if ($rewindRequestCount -ne 0) { $invalidReasons.Add("Snapshot profile unexpectedly contains $rewindRequestCount rewind requests") }
        if ($metadata.RewindScenario -ne 'None') { $invalidReasons.Add("Snapshot profile scenario is $($metadata.RewindScenario), expected None") }
    }
    'RewindMicro' {
        if ($rewindRequestCount -ne 1000) { $invalidReasons.Add("RewindMicro request count is $rewindRequestCount, expected 1000") }
        if ($metadata.RewindScenario -ne 'InterpolatedMiss') { $invalidReasons.Add("RewindMicro scenario is $($metadata.RewindScenario), expected InterpolatedMiss") }
    }
    'RewindLoad' {
        if ($rewindRequestCount -ne 1800) { $invalidReasons.Add("RewindLoad request count is $rewindRequestCount, expected 1800") }
        if ($metadata.RewindScenario -ne 'InterpolatedMiss') { $invalidReasons.Add("RewindLoad scenario is $($metadata.RewindScenario), expected InterpolatedMiss") }
    }
    default { $invalidReasons.Add("unsupported or missing profile: $($metadata.Profile)") }
}

$rewindTimerCount = @($events | Where-Object TimerName -eq 'LyraProfile_RewindTotal').Count
if ($rewindTimerCount -ne $rewindRequestCount) {
    $invalidReasons.Add("rewind timer count ($rewindTimerCount) does not match RewindRequests ($($metadata.RewindRequests))")
}

if ($meshEvents.Count -eq 0) { $invalidReasons.Add("no Lyra mesh game-thread timing events were exported") }
if (@($meshEvents | Where-Object { $_.ThreadName -notmatch '^GameThread' }).Count -gt 0) {
    $invalidReasons.Add("Lyra mesh timing events were observed outside GameThread")
}
if ($workerEvents.Count -eq 0) { $invalidReasons.Add("no native parallel animation worker timing events were exported") }
$completionEvents = @($events | Where-Object {
    $_.TimerName -like '*FParallelAnimationCompletionTask*' -or
    $_.TimerName -eq 'USkeletalMeshComponent_CompleteParallelAnimationEvaluation'
})
if ($completionEvents.Count -eq 0) { $invalidReasons.Add("no native parallel animation completion timing events were exported") }
if (@($completionEvents | Where-Object { $_.ThreadName -notmatch '^GameThread' }).Count -gt 0) {
    $invalidReasons.Add("parallel animation completion executed outside GameThread")
}

$metadata.AnalysisValid = $invalidReasons.Count -eq 0
$metadata.AnalysisInvalidReasons = $invalidReasons -join '; '
$metadata.GetEnumerator() |
    ForEach-Object { [pscustomobject]@{ Metric = $_.Key; Value = $_.Value } } |
    Export-Csv -LiteralPath $metadataPath -NoTypeInformation -Encoding UTF8

Write-Output "Summary: $summaryPath"
Write-Output "Metadata: $metadataPath"
if (-not $metadata.AnalysisValid) {
    Write-Warning "Capture failed validation: $($metadata.AnalysisInvalidReasons)"
}
