#pragma once
#include "HAL/PlatformType.h"
#include "Math/MathUtility.h"
#include "Serialization/Archive.h"

struct FFrameNumber
{
    constexpr FFrameNumber()
        : Value(0)
    {
    }

    constexpr FFrameNumber(int32 InValue)
        : Value(InValue)
    {
    }

    friend FArchive& operator<<(FArchive& Ar, FFrameNumber& FrameNumber);

    bool Serialize(FArchive& Ar);

    FFrameNumber& operator+=(FFrameNumber RHS) { Value += RHS.Value; return *this; }
    FFrameNumber& operator-=(FFrameNumber RHS) { Value -= RHS.Value; return *this; }
    FFrameNumber& operator%=(FFrameNumber RHS) { Value %= RHS.Value; return *this; }

    FFrameNumber& operator++() { ++Value; return *this; }
    FFrameNumber& operator--() { --Value; return *this; }

    FFrameNumber operator++(int32) { FFrameNumber Ret = *this; ++Value; return Ret; }
    FFrameNumber operator--(int32) { FFrameNumber Ret = *this; --Value; return Ret; }

    friend bool operator==(FFrameNumber A, FFrameNumber B) { return A.Value == B.Value; }
    friend bool operator!=(FFrameNumber A, FFrameNumber B) { return A.Value != B.Value; }

    friend bool operator< (FFrameNumber A, FFrameNumber B) { return A.Value < B.Value; }
    friend bool operator> (FFrameNumber A, FFrameNumber B) { return A.Value > B.Value; }
    friend bool operator<=(FFrameNumber A, FFrameNumber B) { return A.Value <= B.Value; }
    friend bool operator>=(FFrameNumber A, FFrameNumber B) { return A.Value >= B.Value; }

    friend FFrameNumber operator+(FFrameNumber A, FFrameNumber B) { return FFrameNumber(A.Value + B.Value); }
    friend FFrameNumber operator-(FFrameNumber A, FFrameNumber B) { return FFrameNumber(A.Value - B.Value); }
    friend FFrameNumber operator%(FFrameNumber A, FFrameNumber B) { return FFrameNumber(A.Value % B.Value); }

    friend FFrameNumber operator-(FFrameNumber A) { return FFrameNumber(-A.Value); }

    friend FFrameNumber operator*(FFrameNumber A, float Scalar) { return FFrameNumber(static_cast<int32>(FMath::Clamp(FMath::FloorToDouble(double(A.Value) * Scalar), (double)MIN_int32, (double)MAX_int32))); }
    friend FFrameNumber operator/(FFrameNumber A, float Scalar) { return FFrameNumber(static_cast<int32>(FMath::Clamp(FMath::FloorToDouble(double(A.Value) / Scalar), (double)MIN_int32, (double)MAX_int32))); }

    friend inline uint32 GetTypeHash(FFrameNumber A)
    {
        return A.Value;
    }

    /**
     * The value of the frame number
     */
    int32 Value;
};
