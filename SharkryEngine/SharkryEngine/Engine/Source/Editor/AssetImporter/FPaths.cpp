#include "FPaths.h"
#include <filesystem>

FString FPaths::GetBaseFilename(const FString& InPath)
{
    std::filesystem::path p(*InPath);
    return FString(p.stem().string().c_str()); // stem = filename without extension
}

FString FPaths::GetExtension(const FString& InPath)
{
    std::filesystem::path p(*InPath);
    return FString(p.extension().string().c_str()).TrimStart(TEXT(".")); // remove leading dot
}

FString FPaths::GetPath(const FString& InPath)
{
    std::filesystem::path p(*InPath);
    return FString(p.parent_path().string().c_str());
}

FString FPaths::Combine(const FString& A, const FString& B)
{
    std::filesystem::path pathA(*A);
    std::filesystem::path pathB(*B);
    std::filesystem::path Combined = pathA / pathB;
    return FString(Combined.string().c_str());
}



FString FPaths::GetFileNameWithoutExtension(const FString& Path)
{
    int32 SlashIdx = Path.Find(TEXT("/"), ESearchCase::IgnoreCase, ESearchDir::FromEnd);
    int32 DotIdx = Path.Find(TEXT("."), ESearchCase::IgnoreCase, ESearchDir::FromEnd);

    if (DotIdx != INDEX_NONE && SlashIdx != INDEX_NONE && DotIdx > SlashIdx)
        return Path.Mid(SlashIdx + 1, DotIdx - SlashIdx - 1);
    else if (SlashIdx != INDEX_NONE)
        return Path.RightChop(SlashIdx + 1);
    return Path;
}

FString FPaths::GetAnimBinaryPath(const FString& SourceFBXPath)
{
    FString FileName = GetFileNameWithoutExtension(SourceFBXPath);
    return TEXT("Contents/Binary/Animation/") + FileName + TEXT(".anim.bin");
}
