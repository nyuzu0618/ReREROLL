// MyGameInstance.cpp
#include "MyGameInstance.h"
#include "MoviePlayer.h"
#include "HAL/PlatformTime.h"
#include "Containers/Ticker.h"

void UMyGameInstance::Init()
{
    Super::Init();

    FCoreUObjectDelegates::PreLoadMap.AddUObject(
        this, &UMyGameInstance::BeginLoadingScreen);
}

void UMyGameInstance::BeginLoadingScreen(const FString& MapName)
{
    UE_LOG(LogTemp, Warning, TEXT("Begin Loading Screen: %s"), *MapName);

    FLoadingScreenAttributes LoadingScreen;

    // マップロードが完了したら自動で終了
    LoadingScreen.bAutoCompleteWhenLoadingCompletes = true;

    // 手動StopMovieを待たない
    LoadingScreen.bWaitForManualStop = false;

    LoadingScreen.bMoviesAreSkippable = false;
    LoadingScreen.PlaybackType = MT_Looped;

    LoadingScreen.MoviePaths.Add(TEXT("LoadingMovie"));

    GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
}
void UMyGameInstance::EndLoadingScreen(UWorld* LoadedWorld)
{
    const double ElapsedTime = FPlatformTime::Seconds() - LoadingStartTime;
    const double RemainingTime = MinimumLoadingTime - ElapsedTime;

    UE_LOG(LogTemp, Warning, TEXT("Loading Elapsed Time: %f"), ElapsedTime);

    if (RemainingTime <= 0.0)
    {
        GetMoviePlayer()->StopMovie();
        UE_LOG(LogTemp, Warning, TEXT("End Loading Screen (Immediate)"));
        return;
    }

    // バックグラウンドスレッドでSleepし、完了後ゲームスレッドでStopMovieを呼ぶ
    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, RemainingTime]()
        {
            FPlatformProcess::Sleep(static_cast<float>(RemainingTime));

            AsyncTask(ENamedThreads::GameThread, [this]()
                {
                    GetMoviePlayer()->StopMovie();
                    UE_LOG(LogTemp, Warning, TEXT("End Loading Screen (Delayed)"));
                });
        });
}