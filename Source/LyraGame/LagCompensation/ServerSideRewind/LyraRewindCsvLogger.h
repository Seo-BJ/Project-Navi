#pragma once

#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"

// 서버 사이드 리와인드 측정 결과를 CSV로 기록하는 단순 로거
// 한 발사(OnTargetDataReadyCallback의 되감기 루프) = CSV 한 줄
class FLyraRewindCsvLogger
{
public:
	static FLyraRewindCsvLogger& Get();

	// 한 발사 단위 측정 결과 기록
	// HitCount: 루프에서 검사된 히트 수, ConfirmedCount: bHitConfirmed가 true였던 수
	// DurationMicroseconds: 되감기 루프 전체 소요 시간 (μs)
	void LogShot(int32 HitCount, int32 ConfirmedCount, double DurationMicroseconds);

	void Shutdown();

private:
	FLyraRewindCsvLogger() = default;
	void EnsureOpened();

	FCriticalSection Mutex;
	class IFileHandle* FileHandle = nullptr;
	bool bHeaderWritten = false;
	int64 ShotIndex = 0;
};
