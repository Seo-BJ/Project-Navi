# Lyra Server Side Rewind profiling

The dedicated server .utrace file is the only runtime measurement artifact. The server does not buffer cycle samples or write CSV files. Export-LyraSSRTrace.ps1 runs Unreal Insights headlessly after capture and creates derived CSV files. Snapshot and mesh code only expose Unreal Insights trace scopes; they do not reference or call the profiling subsystem.

## Editor setup

Use these PIE settings:

- Net Mode: Play As Client
- Number of Players: 1
- Launch Separate Server: enabled
- Run Under One Process: disabled
- Test map contains exactly 10 ALyraLagCompensationTestCharacter actors
- Debug drawing disabled
- Shader, DDC, and asset compilation completed before capture

Enter each command in Editor Preferences > Level Editor > Play > Multiplayer Options > Server > Additional Server Launch Parameters. Do not use the general Additional Launch Parameters field because that targets the client/standalone process.

Create Saved/Profiling/Traces first. Give every run a unique trace filename.

## Run SkeletalMesh and AnimNode separately

`-LyraSSRVariant` selects exactly one implementation for the dedicated-server process. One PIE session cannot measure both variants. For every profile, run the server twice:

1. Run with `-LyraSSRVariant=SkeletalMesh`, wait for the completion log, and stop PIE.
2. Replace only the variant and trace filename with `-LyraSSRVariant=AnimNode`, start a new PIE session, wait for completion, and stop PIE again.

For example, the Snapshot comparison requires these two separate sessions:

~~~text
# Session 1: Skeletal Mesh / AlwaysTickPoseAndRefreshBones
-LyraSSRVariant=SkeletalMesh -LyraSSRProfile=Snapshot -useperfthreads -trace=cpu,frame,task,counters,bookmark,region -tracefile="C:\Users\binij\Documents\UnrealProjects\Navi\Saved\Profiling\Traces\SkeletalMesh_Snapshot_Run1.utrace"

# Stop PIE after completion, replace the server parameters, then start Session 2.
# Session 2: Anim Node / AlwaysTickPose
-LyraSSRVariant=AnimNode -LyraSSRProfile=Snapshot -useperfthreads -trace=cpu,frame,task,counters,bookmark,region -tracefile="C:\Users\binij\Documents\UnrealProjects\Navi\Saved\Profiling\Traces\AnimNode_Snapshot_Run1.utrace"
~~~

The expected result is two independent trace files. Do not put both `-LyraSSRVariant` values in one command line. Underscores in filenames are literal `_` characters; do not write `\_`.

The complete command matrix is below.

~~~text
-LyraSSRVariant=SkeletalMesh -LyraSSRProfile=Snapshot -useperfthreads -trace=cpu,frame,task,counters,bookmark,region -tracefile="C:\Users\binij\Documents\UnrealProjects\Navi\Saved\Profiling\Traces\SkeletalMesh_Snapshot_Run1.utrace"
-LyraSSRVariant=AnimNode -LyraSSRProfile=Snapshot -useperfthreads -trace=cpu,frame,task,counters,bookmark,region -tracefile="C:\Users\binij\Documents\UnrealProjects\Navi\Saved\Profiling\Traces\AnimNode_Snapshot_Run1.utrace"

-LyraSSRVariant=SkeletalMesh -LyraSSRProfile=RewindMicro -useperfthreads -trace=cpu,frame,task,counters,bookmark,region -tracefile="C:\Users\binij\Documents\UnrealProjects\Navi\Saved\Profiling\Traces\SkeletalMesh_RewindMicro_Run1.utrace"
-LyraSSRVariant=AnimNode -LyraSSRProfile=RewindMicro -useperfthreads -trace=cpu,frame,task,counters,bookmark,region -tracefile="C:\Users\binij\Documents\UnrealProjects\Navi\Saved\Profiling\Traces\AnimNode_RewindMicro_Run1.utrace"

-LyraSSRVariant=SkeletalMesh -LyraSSRProfile=RewindLoad -useperfthreads -trace=cpu,frame,task,counters,bookmark,region -tracefile="C:\Users\binij\Documents\UnrealProjects\Navi\Saved\Profiling\Traces\SkeletalMesh_RewindLoad_Run1.utrace"
-LyraSSRVariant=AnimNode -LyraSSRProfile=RewindLoad -useperfthreads -trace=cpu,frame,task,counters,bookmark,region -tracefile="C:\Users\binij\Documents\UnrealProjects\Navi\Saved\Profiling\Traces\AnimNode_RewindLoad_Run1.utrace"
~~~

Repeat every combination three times, changing Run1 to Run2 and Run3.

All profiles retain the five-second warmup. Snapshot and RewindLoad measure for 60 real seconds. RewindMicro performs 50 warmup requests followed by 1,000 measured requests. RewindLoad injects 30 requests per second for 1,800 measured requests. Rewind requests use the existing deterministic interpolated-miss scenario: snapshot lookup, historical pose reconstruction, full physics-asset collision traversal, state restoration, and the final miss decision are all inside the measured request scope.

The benchmark forces the native UE parallel-animation CVars on, but does not replace USkeletalMeshComponent evaluation with a custom worker task. `-useperfthreads` is mandatory because UE 5.5 disables `FApp::ShouldUseThreadingForPerformance()` by default for dedicated servers. Every target mesh tick must expose a native parallel task handle. The benchmark-only ULyraGameEngine::GetMaxTickRate path bypasses NetServerMaxTickRate and sets t.MaxFPS to zero; normal non-profile server runs retain their configured limit.

Wait for this server log before stopping PIE:

~~~text
Lag compensation Insights profile finished.
~~~

Only the spawned dedicated-server process writes the requested .utrace. The editor and client are not measurement targets.

## Export and validate

Run after PIE has stopped and the .utrace file is closed:

~~~powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\Profiling\Export-LyraSSRTrace.ps1 -TraceFile ".\Saved\Profiling\Traces\SkeletalMesh_Snapshot_Run1.utrace"
~~~

The script creates trace-name_analysis beside the trace with:

- summary.csv: average, p50, p95, p99, min, max, total, average total per server frame, frame milliseconds, and effective Hz
- metadata.csv: variant/profile, dedicated-server state, uncapped state, request/result counts, timing-event-derived snapshot/native parallel tick counts, and validity
- timing_events.csv: raw Unreal Insights timing events inside LyraSSR_Measurement
- counter CSV files and unreal_insights_export.log

Discard a run when metadata.csv reports AnalysisValid=False or SessionValid=0.

## Metric source

- Snapshot save cost: LyraProfile_SnapshotUpdateGT
- SSR request cost: LyraProfile_RewindTotal
- Lyra character mesh game-thread tick: LyraProfile_MeshTickGT
- Native worker CPU: the non-game-thread `CharacterMesh0` UObject context scope surrounding `ParallelAnimationEvaluation`
- Native completion: FParallelAnimationCompletionTask and USkeletalMeshComponent_CompleteParallelAnimationEvaluation
- Game-thread wait: USkeletalMeshComponent::BlockOnParallelEvaluationTask
- Server frame time: LyraSSR/FrameMilliseconds (wall-clock interval between dedicated-server ticks)

Use Snapshot captures for snapshot cost, mesh tick, and steady-state server frames. Use RewindMicro for per-request SSR cost. Use RewindLoad for server frames under SSR load.

SnapshotCalls and ParallelAnimationMeshTicks in metadata.csv are derived from timing events inside the measurement region rather than runtime callbacks. ParallelAnimationWorkerCPU excludes same-named game-thread scopes. It is summed CPU work across worker threads, not elapsed frame latency. Do not add it to MeshTickGT and call the result wall time. BlockOnParallelEvaluationTask is wait latency rather than active CPU work, but it remains part of server frame time. Parent inclusive timers and nested child timers must not be summed twice.

The built-in worker timer is server-wide. Keep the map, player count, and every non-target animated actor identical across variants so the A/B delta remains attributable to the selected SSR method. A spectator or non-animated client pawn gives the cleanest worker-thread capture.
