#include "EditorConfigManager.h"

#include <fstream>
#include <sstream>

TMap<FString, FString> FEditorConfigManager::Read() const
{
    TMap<FString, FString> config;
    std::ifstream file(GetData(FilePath));
    std::string line;

    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '[' || line[0] == ';')
        {
            continue;
        }
        std::istringstream ss(line);
        std::string key, value;
        if (std::getline(ss, key, '=') && std::getline(ss, value))
        {
            config[key] = value;
        }
    }
    return config;
}

void FEditorConfigManager::Write() const
{
    std::ofstream file(GetData(FilePath));
    for (const auto& pair : Config)
    {
        file << *pair.Key << "=" << *pair.Value << "\n";
    }
}
