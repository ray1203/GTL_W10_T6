#pragma once
#include <sstream>

#include "Container/Map.h"

// TODO FArchive 상속하여 작동하도록 수정
class FEditorConfigManager
{
public:
    static FEditorConfigManager& GetInstance()
    {
        static FEditorConfigManager Instance;
        return Instance;
    }

   
    TMap<FString, FString> Read() const;
    void Write() const;

    void Clear() { Config.Empty(); }
    
    void AddConfig(const FString& Key, const FString& Value) { Config.Add(Key, Value); }
    void AddConfig(TMap<FString, FString>& InConfig)
    {
        for (auto& [Key, Value] : InConfig)
        {
            if (!Config.Contains(Key))
            {
                Config.Add(Key, Value);
            }
            else
            {
                Config[Key] = Value;
            }
        }
    }

    template <typename T>
    static T GetValueFromConfig(const TMap<FString, FString>& Config, const FString& Key, T DefaultValue) {
        if (const FString* Value = Config.Find(Key))
        {
            std::istringstream iss(**Value);
            const char* a;
            T ConfigValue;
            if (iss >> ConfigValue)
            {
                return ConfigValue;
            }
        }
        return DefaultValue;
    }
    
private:
    TMap<FString, FString> Config;
    const FString FilePath = FString("editor.ini");
};
