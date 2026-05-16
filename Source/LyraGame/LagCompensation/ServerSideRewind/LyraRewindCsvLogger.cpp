#include "LyraRewindCsvLogger.h"

#include "HAL/PlatformFileManager.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "LagCompensation/LyraLagCompProfiler.h"
#include "Misc/Paths.h"
#include "Misc/DateTime.h"

FLyraRewindCsvLogger& FLyraRewindCsvLogger::Get()
{
	static FLyraRewindCsvLogger Instance;
	return Instance;
}

void FLyraRewindCsvLogger::EnsureOpened()
{
	if (FileHandle != nullptr)
	{
		return;
	}

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	const FString Dir = FPaths::ProjectSavedDir() / TEXT("Profiling") / TEXT("RewindCSV");
	PlatformFile.CreateDirectoryTree(*Dir);

	const FString Timestamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
	const FString FilePath = Dir / FString::Printf(TEXT("Rewind_%s.csv"), *Timestamp);

	FileHandle = PlatformFile.OpenWrite(*FilePath, /*bAppend*/ false, /*bAllowRead*/ true);
	if (FileHandle && !bHeaderWritten)
	{
		const FString Header = TEXT("ShotIndex,TimeSeconds,HitCount,ConfirmedCount,FailedCount,bAnyConfirmed,DurationMicroseconds\n");
		const FTCHARToUTF8 Conv(*Header);
		FileHandle->Write(reinterpret_cast<const uint8*>(Conv.Get()), Conv.Length());
		bHeaderWritten = true;
	}
}

void FLyraRewindCsvLogger::LogShot(int32 HitCount, int32 ConfirmedCount, double DurationMicroseconds)
{
	if (FLyraLagCompProfiler::Get().IsActive())
	{
		return;
	}

	FScopeLock Lock(&Mutex);
	EnsureOpened();
	if (!FileHandle)
	{
		return;
	}

	const int32 FailedCount = HitCount - ConfirmedCount;
	const int32 bAnyConfirmed = ConfirmedCount > 0 ? 1 : 0;
	const double TimeSeconds = FPlatformTime::Seconds();

	const FString Line = FString::Printf(
		TEXT("%lld,%.6f,%d,%d,%d,%d,%.3f\n"),
		++ShotIndex, TimeSeconds, HitCount, ConfirmedCount, FailedCount, bAnyConfirmed, DurationMicroseconds);

	const FTCHARToUTF8 Conv(*Line);
	FileHandle->Write(reinterpret_cast<const uint8*>(Conv.Get()), Conv.Length());
	FileHandle->Flush();
}

void FLyraRewindCsvLogger::Shutdown()
{
	FScopeLock Lock(&Mutex);
	if (FileHandle)
	{
		FileHandle->Flush();
		delete FileHandle;
		FileHandle = nullptr;
	}
}
