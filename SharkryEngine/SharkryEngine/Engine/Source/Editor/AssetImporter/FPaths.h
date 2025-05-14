#pragma once

#include "Define.h"
#include <string>

class FPaths
{
public:
    // 예: "C:/Assets/Walk_Run.fbx" → "Walk_Run"
    static FString GetBaseFilename(const FString& InPath);

    // 예: "C:/Assets/Walk_Run.fbx" → "fbx"
    static FString GetExtension(const FString& InPath);

    // 예: "C:/Assets/Walk_Run.fbx" → "C:/Assets"
    static FString GetPath(const FString& InPath);

    // 예: Combine("C:/Assets", "Walk_Run.anim.bin") → "C:/Assets/Walk_Run.anim.bin"
    static FString Combine(const FString& A, const FString& B);

    static FString GetFileNameWithoutExtension(const FString& Path);

    static FString GetAnimBinaryPath(const FString& SourceFBXPath);
};
