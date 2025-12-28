// Fill out your copyright notice in the Description page of Project Settings.


#include "StreamLevelLoad.h"
#include "Engine/LevelStreaming.h"
#include "Kismet/GameplayStatics.h"


UStreamLevelLoad::UStreamLevelLoad()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UStreamLevelLoad::BeginPlay()
{
	Super::BeginPlay();

	if(!bIsRunningOnServer)
		return;
	
	ActivateLevelIndex = StartLoadLevelIndex;
	// LoadStreamingLevel(LevelNames[StartLoadLevelIndex]);
}

void UStreamLevelLoad::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UStreamLevelLoad::LoadStreamingLevel(FName LevelName)
{
	// if(!LevelNames.Contains(LevelName))
	// {
	// 	UE_LOG(LogTemp, Error, TEXT("Level %s Not Found"), *LevelName.ToString());
	// 	return;
	// }

	FLatentActionInfo LatentActionInfo = FLatentActionInfo(1, 1, TEXT("StreamLevelLoadCompleted"), this);
	LoadStreamLevel(LevelName, LatentActionInfo);
}

void UStreamLevelLoad::UnLoadStreamingLevel(FName LevelName)
{
	FName UnLoadLevelName = LevelName;
	if(!LevelNames.Contains(LevelName))
		UnLoadLevelName = CurrentLevelName;
	UnloadStreamLevel(UnLoadLevelName ,FLatentActionInfo(2, 1, TEXT("StreamLevelUnloadCompleted"), this));
}

void UStreamLevelLoad::InitLevelStart()
{
	CurrentLevelName = NextLevelName = LevelNames[StartLoadLevelIndex];
}


void UStreamLevelLoad::ActivateStreamingLevel()
{
	if(!bIsRunningOnServer)
		return;

	if (bIsAlwaysLoadedLevel)
	{
		CurrentLevelName = TEXT("AlwaysLoadedLevel");
		if (OnLevelActivated.IsBound())
			OnLevelActivated.Broadcast(CurrentLevelName);
		return;
	}

	if(bIsLoadAllLevel)
	{
		if(bIsActiveAllLevel)
		{
			for(FName levelName : LevelNames)
				HideStreamingLevel(StreamingLevels[levelName]);
		}
		else
		{
			HideStreamingLevel(CurrentStreamingLevel);
			CurrentStreamingLevel = UGameplayStatics::GetStreamingLevel(GetWorld(), NextLevelName);
			
			CurrentLevelName = NextLevelName;
			int32 nextActivateLevelIndex = LevelNames.Find(CurrentLevelName) + 1;
			if(nextActivateLevelIndex < LevelNames.Num())
				NextLevelName = LevelNames[nextActivateLevelIndex];
			
			// ShowStreamingLevel(CurrentStreamingLevel);
			if (ShowLevelDelayTime == 0)
			{
				ShowStreamingLevel(CurrentStreamingLevel);
				if(OnLevelActivated.IsBound())
					OnLevelActivated.Broadcast(CurrentLevelName);
			}else
			{
				//延迟显示
				GetWorld()->GetTimerManager().SetTimer(TimerHandle_ShowNextLevel, [this]()
				{
					ShowStreamingLevel(CurrentStreamingLevel);
					if(OnLevelActivated.IsBound())
						OnLevelActivated.Broadcast(CurrentLevelName);
				}, ShowLevelDelayTime, false);
			}

			return;
		}
	}
	else
	{
		CurrentStreamingLevel = StreamingLevels[CurrentLevelName];
		ShowStreamingLevel(CurrentStreamingLevel);
	}
	
	if(OnLevelActivated.IsBound())
		OnLevelActivated.Broadcast(CurrentLevelName);
}

void UStreamLevelLoad::ShowStreamingLevel(ULevelStreaming* LevelStreaming)
{
	if (LevelStreaming && !LevelStreaming->IsLevelVisible())
	{
		LevelStreaming->SetShouldBeLoaded(true);
		LevelStreaming->SetShouldBeVisible(true);
	}
}

void UStreamLevelLoad::HideStreamingLevel(ULevelStreaming* LevelStreaming)
{
	if (LevelStreaming && LevelStreaming->IsLevelVisible())
	{
		LevelStreaming->SetShouldBeLoaded(false);
		LevelStreaming->SetShouldBeVisible(false);
	}
}

void UStreamLevelLoad::LoadStreamLevel(FName LevelName, FLatentActionInfo LatentActionInfo)
{
	CurrentLevelName = LevelName;
	if(OnLevelLoadBefore.IsBound())
		OnLevelLoadBefore.Broadcast(CurrentLevelName);

	
	UGameplayStatics::LoadStreamLevel(GetWorld(), LevelName, true, true, LatentActionInfo);
}

void UStreamLevelLoad::UnloadStreamLevel(FName LevelName, FLatentActionInfo LatentActionInfo)
{
	
	if(OnUnLevelLoadBefore.IsBound())
		OnUnLevelLoadBefore.Broadcast(LevelName);
	
	UGameplayStatics::UnloadStreamLevel(GetWorld(), LevelName, LatentActionInfo, true);
	return;


	
	if (LevelName == TEXT("AlwaysLoadedLevel"))
	{
		if(!bIsLoadAllLevel)
		{
			int32 nextActivateLevelIndex = LevelNames.Find(CurrentLevelName) + 1;
			if(nextActivateLevelIndex < LevelNames.Num())
				NextLevelName = LevelNames[nextActivateLevelIndex];
			LoadStreamingLevel(NextLevelName);
		}
		return;
	}
	
	if(!LevelNames.Contains(LevelName))
	{
		UE_LOG(LogTemp, Error, TEXT("Level %s Not Found"), *LevelName.ToString());
		return;
	}
	
	if(OnUnLevelLoadBefore.IsBound())
		OnUnLevelLoadBefore.Broadcast(LevelName);
	

	if(bIsLoadAllLevel)
	{
		if(OnUnLevelLoaded.IsBound())
				OnUnLevelLoaded.Broadcast(LevelName);
	}
	else
	{
		UGameplayStatics::UnloadStreamLevel(GetWorld(), LevelName, LatentActionInfo, true);		
	}
}

void UStreamLevelLoad::StreamLevelLoadCompleted()
{
	if(OnLevelLoaded.IsBound())
		OnLevelLoaded.Broadcast(CurrentLevelName);
	return;

	
	ULevelStreaming* StreamingLevel = UGameplayStatics::GetStreamingLevel(GetWorld(), CurrentLevelName);	
	if(StreamingLevels.Contains(CurrentLevelName))
		StreamingLevels[CurrentLevelName] = StreamingLevel;
	else
		StreamingLevels.Add(CurrentLevelName, StreamingLevel);
	
	
	int32 nextLevelIndex = LevelNames.Find(CurrentLevelName) + 1;
	if(LevelNames.Num() > nextLevelIndex)
	{
		NextLevelName = LevelNames[nextLevelIndex];
		if(OnLevelLoaded.IsBound())
			OnLevelLoaded.Broadcast(CurrentLevelName);
		 
		if(bIsLoadAllLevel)
			LoadStreamingLevel(NextLevelName);
		else
		{
			if (nextLevelIndex > 1)
				ActivateStreamingLevel();
		}
	}
	else
	{
		if(!bIsLoadAllLevel)
		{
			if(OnLevelLoaded.IsBound())
				OnLevelLoaded.Broadcast(CurrentLevelName);
			return;
		}
		
		CurrentLevelName = NextLevelName = LevelNames[StartLoadLevelIndex];
		if(OnLevelLoadedAll.IsBound())
			OnLevelLoadedAll.Broadcast(LevelNames);
	}
}

void UStreamLevelLoad::StreamLevelUnloadCompleted()
{
	if(OnUnLevelLoaded.IsBound())
		OnUnLevelLoaded.Broadcast(CurrentLevelName);
	return;


	if(!bIsLoadAllLevel && !bIsAlwaysLoadedLevel)
	{
		int32 nextActivateLevelIndex = LevelNames.Find(CurrentLevelName) + 1;
		if(nextActivateLevelIndex < LevelNames.Num())
			NextLevelName = LevelNames[nextActivateLevelIndex];
		LoadStreamingLevel(NextLevelName);
	}
}


void UStreamLevelLoad::ResetStreamingLevel()
{
	if(bIsLoadAllLevel)
	{
		if(!bIsActiveAllLevel)
		{
			if (CurrentStreamingLevel != nullptr)
			{
				//必须设置 ShouldBeLoaded 为 false，否则再次激活地图时会造成内容丢失而崩溃
				CurrentStreamingLevel->SetShouldBeLoaded(false);
				CurrentStreamingLevel->SetShouldBeVisible(false);
				UE_LOG(LogTemp, Log, TEXT("ResetStreamingLevel： %s"), *CurrentLevelName.ToString());
			}
		}
	}
	else
	{
		if (CurrentStreamingLevel != nullptr)
		{
			// CurrentStreamingLevel->SetShouldBeLoaded(false);
			// CurrentStreamingLevel->SetShouldBeVisible(false);

			// Unload StreamingLevel
			if(OnUnLevelLoadBefore.IsBound())
				OnUnLevelLoadBefore.Broadcast(CurrentLevelName);			
			UGameplayStatics::UnloadStreamLevel(GetWorld(), CurrentLevelName, FLatentActionInfo(2, 1, TEXT("StreamLevelUnloadCompleted"), this), true);	
		}
	}
	CurrentLevelName = NextLevelName = LevelNames[StartLoadLevelIndex];
}

void UStreamLevelLoad::PreloadAllAnimationAssets()
{
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Actors);
	for (AActor* Actor : Actors)
	{
		TArray<USkeletalMeshComponent*> Components;
		Actor->GetComponents<USkeletalMeshComponent>(Components);
		for (USkeletalMeshComponent* Component : Components)
		{
			if (!Component || !Component->GetSkeletalMeshAsset())
				continue;
			
			USkeleton* Skeleton = Component->GetSkeletalMeshAsset()->GetSkeleton();
			if (Skeleton)
				Skeleton->ConditionalPostLoad();
		}
	}
}


