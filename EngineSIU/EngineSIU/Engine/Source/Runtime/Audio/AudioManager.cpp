#include "AudioManager.h"

#include "UObject/Object.h"

std::unordered_map<EAudioType, FString> AudioManager::AudioMap = {
    { EAudioType::Mario, "Contents/Audio/mario.mp3" },
    { EAudioType::Goofy, "Contents/Audio/goofy-ahh-car-horn.mp3"},
    { EAudioType::MainTheme, "Contents/Audio/LastWar_Main.mp3" }
};

void AudioManager::Initialize()
{
    unsigned int Version;
    FMOD_RESULT result = FMOD::System_Create(&System);

    if (result != FMOD_OK)
    {
        MessageBox(nullptr, L"failed! System Create", L"Error", MB_ICONERROR | MB_OK);
    }
    result = System->getVersion(&Version);
    if (result != FMOD_OK)
    {
        MessageBox(nullptr, L"failed! Get Version", L"Error", MB_ICONERROR | MB_OK);
    }
    else
    {
        UE_LOG(LogLevel::Display, "FMOD version %08x", Version);
    }

    result = System->init(1024, FMOD_INIT_NORMAL, nullptr);
    if (result != FMOD_OK)
    {
        MessageBox(nullptr, L"failed! System Init (Channel)", L"Error", MB_ICONERROR | MB_OK);
    }

    // 채널 그룹 생성
    System->createChannelGroup("BGM_Group", &BgmGroup);
    System->createChannelGroup("SFX_Group", &SfxGroup);
    
    BgmGroup->setVolume(0.75f);
    SfxGroup->setVolume(0.75f);

    // 테스트용 코드
    // FSlateAppMessageHandler* Handler = GEngineLoop.GetAppMessageHandler();
    // Handler->OnKeyDownDelegate.AddLambda([this](const FKeyEvent& InKeyEvent)
    // {
    //     switch (InKeyEvent.GetCharacter())
    //     {
    //     case 'Q':
    //         {
    //             if (InKeyEvent.GetInputEvent() == IE_Pressed)
    //             {
    //                 PlayBgm(EAudioType::Mario);
    //             }
    //             break;
    //         }
    //     case 'W':
    //         {
    //             if (InKeyEvent.GetInputEvent() == IE_Pressed)
    //             {
    //                 PlayBgm(EAudioType::Goofy);
    //             }
    //             break;
    //         }
    //     case 'E':
    //         {
    //             if (InKeyEvent.GetInputEvent() == IE_Pressed)
    //             {
    //                 StopBgm();
    //             }
    //             break;
    //         }
    //     case 'A':
    //         {
    //             if (InKeyEvent.GetInputEvent() == IE_Pressed)
    //             {
    //                 PlayOneShot(EAudioType::Mario);
    //             }
    //             break;
    //         }
    //     case 'S':
    //         {
    //             if (InKeyEvent.GetInputEvent() == IE_Pressed)
    //             {
    //                 PlayOneShot(EAudioType::Goofy);
    //             }
    //             break;
    //         }
    //     default:
    //         break;
    //     }
    // });
}

void AudioManager::Release()
{
    for (const auto& [_, Map] : SoundMap)
    {
        for (const auto& [_, Map2] : Map)
        {
            for (const auto& [_, Sound] : Map2)
            {
                Sound->release();
            } 
        } 
    }
    SoundMap.Empty();
    System->release();
}

void AudioManager::PlayOneShot(EAudioType AudioType)
{
    FMOD::Channel* Channel = nullptr;

    FMOD::Sound* Sound = GetSound(AudioMap[AudioType], FMOD_DEFAULT | FMOD_CREATESAMPLE, nullptr);

    // 사운드 재생
    FMOD_RESULT result = System->playSound(Sound, SfxGroup, false, &Channel);
    if (result != FMOD_OK)
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to play sound!"));
    }
}

void AudioManager::PlayBgm(EAudioType AudioType)
{
    if (BgmChannel != nullptr)
    {
        StopBgm();
    }
    
    FMOD::Sound* Sound = GetSound(AudioMap[AudioType], FMOD_LOOP_NORMAL | FMOD_CREATESAMPLE, nullptr);

    // 사운드 재생
    FMOD_RESULT result = System->playSound(Sound, BgmGroup, false, &BgmChannel);
    if (result != FMOD_OK)
    {
        UE_LOG(LogLevel::Error, TEXT("Failed to play sound!"));
    }
}

void AudioManager::StopBgm()
{
    BgmChannel->stop();
    BgmChannel = nullptr;
}


void AudioManager::Tick()
{
    FMOD_RESULT result = System->update();                  
    if (result != FMOD_OK)
    {
        MessageBox(nullptr, L"System Update Error", L"Error", MB_ICONERROR | MB_OK);
    }
}

FMOD::Sound* AudioManager::GetSound(const FString& SoundPath, FMOD_MODE Mode, FMOD_CREATESOUNDEXINFO* ExInfo)
{
    if (!SoundMap.Contains(SoundPath))
    {
        SoundMap.Add(SoundPath, TMap<FMOD_MODE, TMap<FMOD_CREATESOUNDEXINFO*, FMOD::Sound*>>());
    }

    if (!SoundMap[SoundPath].Contains(Mode))
    {
        SoundMap[SoundPath].Add(Mode, TMap<FMOD_CREATESOUNDEXINFO*, FMOD::Sound*>());
    }

    if (!SoundMap[SoundPath][Mode].Contains(ExInfo))
    {
        FMOD::Sound* Sound = nullptr;
        System->createSound(GetData(SoundPath), Mode, ExInfo, &Sound);
        SoundMap[SoundPath][Mode].Add(ExInfo, Sound);
    }
    
    return SoundMap[SoundPath][Mode][ExInfo];
}

AudioManager& AudioManager::Get()
{
    static AudioManager Instance;
    return Instance;
}
