#include "Time/TimeSubsystem.h"

#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogTime, Log, All);

void UTimeSubsystem::AdvanceDay()
{
	++CurrentDay;

	UE_LOG(LogTime, Log, TEXT("AdvanceDay: now Day %d"), CurrentDay);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			static_cast<int32>(GetUniqueID()),
			2.0f,
			FColor::Cyan,
			FString::Printf(TEXT("New Day: %d"), CurrentDay));
	}

	OnNewDay.Broadcast(CurrentDay);
}