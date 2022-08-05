// Fill out your copyright notice in the Description page of Project Settings.


#include "PatchDownloaderSubSystem.h"

#include "ChunkDownLoader.h"
#include "Misc/CoreDelegates.h"
#include "AssetRegistryModule.h"

void UPatchDownloaderSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
}
void UPatchDownloaderSubSystem::Deinitialize()
{
	UE_LOG(LogTemp, Display, TEXT("ChunkDownloaderShutDown"));
	FChunkDownloader::Shutdown();
	Super::Deinitialize();

}

void UPatchDownloaderSubSystem::OnManifestUpdateComplete(bool bSuccess)
{
	UE_LOG(LogTemp, Display, TEXT("OnManifestUpdateComplete:%b"),bSuccess);
	bIsDownloadManifestUpToDate = bSuccess;
	if (bIsDownloadManifestUpToDate)
	{
		PatchGame();
	}
	OnManifestComplete.Broadcast(bSuccess);
}

void UPatchDownloaderSubSystem::OnDownloadComplete(bool bSuccess)
{
	if (bSuccess)
	{
		UE_LOG(LogTemp, Display, TEXT("Download complete"));

		// get the chunk downloader
		TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

		FJsonSerializableArrayInt DownloadedChunks;

		for (int32 ChunkID : ChunkDownloadList)
		{
			DownloadedChunks.Add(ChunkID);
		}

		//Mount the chunks
		TFunction<void(bool bSuccess)> MountCompleteCallback = [&](bool bSuccess) {OnMountComplete(bSuccess); };
		Downloader->MountChunks(DownloadedChunks, MountCompleteCallback);

		//OnPatchComplete.Broadcast(true);

	}
	else
	{

		UE_LOG(LogTemp, Display, TEXT("Load process failed"));

		// call the delegate
		OnPatchComplete.Broadcast(false);
	}
}

void UPatchDownloaderSubSystem::OnLoadingModeComplete(bool bSuccess)
{
	OnDownloadComplete(bSuccess);
}

void UPatchDownloaderSubSystem::OnMountComplete(bool bSuccess)
{
	OnPatchComplete.Broadcast(bSuccess);
}

void UPatchDownloaderSubSystem::GetLoadingProgress(int32& FilesDownloaded, int32& TotalFilesToDownload, float& DownloadPercent, int32& ChunksMounted, int32& TotalChunksToMount, float& MountPercent) const
{
	//Get a reference to ChunkDownloader
	TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

	//Get the loading stats struct
	FChunkDownloader::FStats LoadingStats = Downloader->GetLoadingStats();

	//Get the bytes downloaded and bytes to download
	FilesDownloaded = LoadingStats.BytesDownloaded;
	TotalFilesToDownload = LoadingStats.TotalBytesToDownload;

	//Get the number of chunks mounted and chunks to download
	ChunksMounted = LoadingStats.ChunksMounted;
	TotalChunksToMount = LoadingStats.TotalChunksToMount;

	//Calculate the download and mount percent using the above stats
	DownloadPercent = (float)FilesDownloaded / (float)TotalFilesToDownload;
	MountPercent = (float)ChunksMounted / (float)TotalChunksToMount;
}

bool UPatchDownloaderSubSystem::PatchGame()
{
	// make sure the download manifest is up to date
	if (bIsDownloadManifestUpToDate)
	{
		// get the chunk downloader
		TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

		// report current chunk status
		for (int32 ChunkID : ChunkDownloadList)
		{
			int32 ChunkStatus = static_cast<int32>(Downloader->GetChunkStatus(ChunkID));
			UE_LOG(LogTemp, Display, TEXT("Show ChunkState______Chunk %i status: %i"), ChunkID, ChunkStatus);
		}

		TFunction<void(bool bSuccess)> DownloadCompleteCallback = [&](bool bSuccess) {OnDownloadComplete(bSuccess); };
		Downloader->DownloadChunks(ChunkDownloadList, DownloadCompleteCallback, 1);

		// start loading mode
		TFunction<void(bool bSuccess)> LoadingModeCompleteCallback = [&](bool bSuccess) {OnLoadingModeComplete(bSuccess); };
		Downloader->BeginLoadingMode(LoadingModeCompleteCallback);
		return true;
	}

	// we couldn't contact the server to validate our manifest, so we can't patch
	UE_LOG(LogTemp, Display, TEXT("Manifest Update Failed. Can't patch the game"));

	return false;
}

bool UPatchDownloaderSubSystem::CreatePatchDownloader()
{
	UE_LOG(LogTemp, Warning, TEXT("CreatePatchDownloader"));
	const FString DeploymentName = "PatchingDemoLive";
	const FString ContentBuildId = "PatcherKey";
	TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetOrCreate();
	Downloader->Initialize("Windows", 8);
	Downloader->LoadCachedBuild(DeploymentName);

	TFunction<void(bool bSuccess)> UpdateCompleteCallback = [&](bool bSuccess) {OnManifestUpdateComplete(bSuccess); };
	Downloader->UpdateBuild(DeploymentName, ContentBuildId, UpdateCompleteCallback);
	return false;
}
