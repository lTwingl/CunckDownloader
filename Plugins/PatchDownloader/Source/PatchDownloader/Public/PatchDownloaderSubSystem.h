// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PatchDownloaderSubSystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPatchCompleteDelegate, bool, Succeeded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FManifestCompleteDelegate, bool, Succeeded);

/**
 * 
 */
UCLASS()
class PATCHDOWNLOADER_API UPatchDownloaderSubSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	

public:
	/** Implement this for initialization of instances of the system */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Implement this for deinitialization of instances of the system */
	virtual void Deinitialize() override;
protected:
	UPROPERTY()
		bool bIsDownloadManifestUpToDate;

	UFUNCTION()
		void OnManifestUpdateComplete(bool bSuccess);


	UPROPERTY(EditDefaultsOnly, Category = "Patching")
		TArray<int32> ChunkDownloadList = {10001,10002};


	/** Called when the chunk download process finishes */
	void OnDownloadComplete(bool bSuccess);

	/** Called whenever ChunkDownloader's loading mode is finished*/
	void OnLoadingModeComplete(bool bSuccess);

	/** Called when ChunkDownloader finishes mounting chunks */
	void OnMountComplete(bool bSuccess);
public:

	UFUNCTION(BlueprintPure)
		void GetLoadingProgress(int32& FilesDownloaded, int32& TotalFilesToDownload, float& DownloadPercent, int32& ChunksMounted, int32& TotalChunksToMount, float& MountPercent) const;

	UPROPERTY(BlueprintAssignable, Category = "Patching");
	FPatchCompleteDelegate OnPatchComplete;
	UPROPERTY(BlueprintAssignable, Category = "Patching");
	FManifestCompleteDelegate OnManifestComplete;


	
	/** Starts the game patching process. Returns false if the patching manifest is not up to date. */
	UFUNCTION(BlueprintCallable, Category = "Patching")
		bool PatchGame();

	/** Starts the game patching process. Returns false if the patching manifest is not up to date. */
	UFUNCTION(BlueprintCallable, Category = "PatchDownload")
		bool CreatePatchDownloader();
};


