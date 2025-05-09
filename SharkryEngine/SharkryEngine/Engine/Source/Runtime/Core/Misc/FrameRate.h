#pragma once

#include "Engine/Source/Runtime/Core/HAL/PlatformType.h"

struct FFrameRate
{
    int32 Denominator;
    int32 Numerator;
    FFrameRate()
        : Numerator(60000)
        , Denominator(1)
    {
    }

    /** Construct a framerate with the given numerator and denominator. */
    FFrameRate(uint32 InNumerator, uint32 InDenominator)
        : Numerator(InNumerator)
        , Denominator(InDenominator)
    {
    }

    /** Get the decimal representation of this framerate (frames/sec). */
    FORCEINLINE double AsDecimal() const
    {
        return Denominator != 0
            ? static_cast<double>(Numerator) / static_cast<double>(Denominator)
            : 0.0;
    }

    // TODO 아래 함수 구현
    /** Convert a time in seconds to an integral frame number (floor). */
    /*FFrameNumber AsFrameNumber(double InTimeSeconds) const
    {
        return FFrameNumber(int32(FMath::FloorToDouble(InTimeSeconds * AsDecimal())));
    }*/

    // TODO 아래 함수 구현
    /** Convert a time in seconds to an FFrameTime (supports sub-frame). */
    /*FFrameTime AsFrameTime(double InTimeSeconds) const
    {
        return FFrameTime(AsFrameNumber(InTimeSeconds));
    }*/

    /** Get the interval between frames in seconds (1 / framerate). */
    FORCEINLINE double AsInterval() const
    {
        return Denominator != 0
            ? static_cast<double>(Denominator) / static_cast<double>(Numerator)
            : 0.0;
    }

    // TODO 아래 함수 구현
    /** Convert a frame number to seconds at this framerate. */
    /*double AsSeconds(FFrameTime FrameNumber) const
    {
        return FrameNumber.AsDecimal() / AsDecimal();
    }*/

    /** Returns true if this framerate is valid (Denominator > 0, Numerator > 0). */
    bool IsValid() const
    {
        return Denominator > 0 && Numerator > 0;
    }

    /** Get the reciprocal framerate (swap numerator and denominator). */
    FFrameRate Reciprocal() const
    {
        return FFrameRate(Denominator, Numerator);
    }

    // TODO 아래 함수 구현
    /** Serialize to/from an FArchive. */
    /*bool Serialize(FArchive& Ar)
    {
        Ar << Numerator << Denominator;
        return true;
    }*/

    // TODO 아래 함수 구현
    /** Produce a human-friendly text, e.g. "30 fps". */
    //FText ToPrettyText() const;

    ///** Snap a time from one rate to another. */
    //static FFrameTime Snap(FFrameTime SourceTime, FFrameRate SourceRate, FFrameRate SnapToRate);

    ///** Transform a time from one rate to another. */
    //static FFrameTime TransformTime(FFrameTime SourceTime, FFrameRate SourceRate, FFrameRate DestinationRate);
};
