// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "StreamLevelLoad.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UStreamLevelLoad : public UActorComponent
{
	GENERATED_BODY()


	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelLoadBefore, FName, LevelName);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnLevelLoadBefore, FName, LevelName);
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelActivated, FName, LevelName);
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelLoaded, FName, LevelName);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnLevelLoaded, FName, LevelName);
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelLoadedAll, TArray<FName>, LevelName);

public:
	// Sets default values for this component's properties
	UStreamLevelLoad();

	UPROPERTY(BlueprintAssignable)
	FOnLevelLoadBefore OnLevelLoadBefore;

	UPROPERTY(BlueprintAssignable)
	FOnUnLevelLoadBefore OnUnLevelLoadBefore;
	
	UPROPERTY(BlueprintAssignable)
	FOnLevelActivated OnLevelActivated;
	
	UPROPERTY(BlueprintAssignable)
	FOnLevelLoaded OnLevelLoaded;

	UPROPERTY(BlueprintAssignable)
	FOnUnLevelLoaded OnUnLevelLoaded;
	
	UPROPERTY(BlueprintAssignable)
	FOnLevelLoadedAll OnLevelLoadedAll;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLoadAllLevel;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsActiveAllLevel;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsRunningOnServer;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAlwaysLoadedLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsShowLoadingScreen = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPreloadSkeletalAsset = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FName> LevelNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StartLoadLevelIndex = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ShowLevelDelayTime = 0.1f;

	UPROPERTY(BlueprintReadOnly)
	int32 ActivateLevelIndex = 0;

	UPROPERTY(BlueprintReadOnly)
	TMap<FName, ULevelStreaming*> StreamingLevels;

	UPROPERTY(BlueprintReadOnly)
	FName CurrentLevelName;

	UPROPERTY(BlueprintReadOnly)
	FName NextLevelName;

	UPROPERTY(BlueprintReadOnly)
	ULevelStreaming* CurrentStreamingLevel;
	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	
	FTimerHandle TimerHandle_ShowNextLevel;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void LoadStreamingLevel(FName LevelName);
	
	UFUNCTION(BlueprintCallable)
	void UnLoadStreamingLevel(FName LevelName);

	UFUNCTION(BlueprintCallable)
	void InitLevelStart();

	UFUNCTION(BlueprintCallable)
	void ActivateStreamingLevel();

	UFUNCTION(BlueprintCallable)
	bool IsRunningOnServer();

	UFUNCTION(BlueprintCallable)
	void ResetStreamingLevel();

	UFUNCTION(BlueprintCallable)
	void PreloadAllAnimationAssets();

private:
	
	UFUNCTION()
	void LoadStreamLevel(FName LevelName, FLatentActionInfo LatentActionInfo);

	UFUNCTION()
	void UnloadStreamLevel(FName LevelName, FLatentActionInfo LatentActionInfo);

	UFUNCTION()
	void ShowStreamingLevel(ULevelStreaming* LevelStreaming);

	UFUNCTION()
	void HideStreamingLevel(ULevelStreaming* LevelStreaming);

	UFUNCTION()
	void StreamLevelLoadCompleted();

	UFUNCTION()
	void StreamLevelUnloadCompleted();
};
