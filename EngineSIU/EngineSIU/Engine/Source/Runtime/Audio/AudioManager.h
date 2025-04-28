#pragma once
#include "FMOD/include/fmod.hpp"

#include "Container/Map.h"
#include "Container/String.h"

enum class EAudioType{
    Mario,
    Goofy
};

class AudioManager
{
public:
    static AudioManager& Get();

    void Initialize();
    void Release();
    void Tick();

    void PlayOneShot(EAudioType AudioType);

    void PlayBgm(EAudioType AudioType);
    void StopBgm();


private:
    FMOD::Sound* GetSound(const FString& SoundPath, FMOD_MODE Mode = FMOD_DEFAULT, FMOD_CREATESOUNDEXINFO* ExInfo = nullptr);

private:
    static std::unordered_map<EAudioType, FString> AudioMap;
    
    FMOD::Channel* BgmChannel = nullptr;

    FMOD::System* System = nullptr;

    FMOD::ChannelGroup* BgmGroup = nullptr;
    FMOD::ChannelGroup* SfxGroup = nullptr;

    TMap<FString, TMap<FMOD_MODE, TMap<FMOD_CREATESOUNDEXINFO*, FMOD::Sound*>>> SoundMap;
};
