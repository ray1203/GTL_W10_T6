#pragma once
#include <cmath>
#include <numbers>
#include "Core/HAL/PlatformType.h"
#include <random>


#define PI                   (3.1415926535897932f)
#define SMALL_NUMBER         (1.e-8f)
#define KINDA_SMALL_NUMBER   (1.e-4f)

#define PI_DOUBLE            (3.141592653589793238462643383279502884197169399)

struct FMath
{
    /** A와 B중에 더 작은 값을 반환합니다. */
    template <typename T>
    [[nodiscard]] static FORCEINLINE constexpr T Min(const T A, const T B)
    {
        return A < B ? A : B;
    }

    /** A와 B중에 더 큰 값을 반환합니다. */
    template <typename T>
    [[nodiscard]] static FORCEINLINE constexpr T Max(const T A, const T B)
    {
        return B < A ? A : B;
    }

    /** A, B, C 중에 가장 큰 값을 반환합니다. */
    template <typename T>
    [[nodiscard]] static FORCEINLINE constexpr T Max3(const T A, const T B, const T C)
    {
        return Max(A, Max(B, C));
    }

    /** X를 Min과 Max의 사이의 값으로 클램핑 합니다. */
    template <typename T>
    [[nodiscard]] static FORCEINLINE constexpr T Clamp(const T X, const T MinValue, const T MaxValue)
    {
        return Max(Min(X, MaxValue), MinValue);
    }

    /** A의 절댓값을 구합니다. */
    template <typename T>
    [[nodiscard]] static FORCEINLINE constexpr T Abs(const T A)
    {
        return A < T(0) ? -A : A;
    }

    /** Returns 1, 0, or -1 depending on relation of T to 0 */
    template< class T >
    static constexpr FORCEINLINE T Sign(const T A)
    {
        return (A > (T)0) ? (T)1 : ((A < (T)0) ? (T)-1 : (T)0);
    }

    /** A의 제곱을 구합니다. */
    template <typename T>
    [[nodiscard]] static FORCEINLINE constexpr T Pow(const T A)
    {
        return A * A;
    }

    template <typename T>
    [[nodiscard]] static FORCEINLINE constexpr T Pow(const T A, const T B)
    {
        return pow(A, B);
    }


    // A의 제곱근을 구합니다.
    [[nodiscard]] static FORCEINLINE float Sqrt(float A) { return sqrtf(A); }
    [[nodiscard]] static FORCEINLINE double Sqrt(double A) { return sqrt(A); }

    /** A의 역제곱근을 구합니다. */
    [[nodiscard]] static FORCEINLINE float InvSqrt(float A) { return 1.0f / sqrtf(A); }
    [[nodiscard]] static FORCEINLINE double InvSqrt(double A) { return 1.0 / sqrt(A); }

    /** A와 B를 Alpha값에 따라 선형으로 보간합니다. */
    template <typename T>
    [[nodiscard]] static FORCEINLINE constexpr T Lerp(const T& A, const T& B, float Alpha)
    {
        return static_cast<T>((A * (1.0f - Alpha)) + (B * Alpha));
    }

    /** A와 B를 Alpha값에 따라 선형으로 보간합니다. */
    template <typename T>
    [[nodiscard]] static FORCEINLINE constexpr T Lerp(const T& A, const T& B, double Alpha)
    {
        return static_cast<T>((A * (1.0 - Alpha)) + (B * Alpha));
    }

    template <
        typename T,
        typename U
    >
    [[nodiscard]] static constexpr T CubicInterp(const T& P0, const T& T0, const T& P1, const T& T1, const U& A)
    {
        const U A2 = A * A;
        const U A3 = A2 * A;

        return T((((2 * A3) - (3 * A2) + 1) * P0) + ((A3 - (2 * A2) + A) * T0) + ((A3 - A2) * T1) + (((-2 * A3) + (3 * A2)) * P1));
    }

    template <typename T>
    [[nodiscard]] static FORCEINLINE constexpr auto RadiansToDegrees(const T& RadVal) -> decltype(RadVal* (180.0f / PI))
    {
        return RadVal * (180.0f / PI);
    }

    [[nodiscard]] static FORCEINLINE constexpr float RadiansToDegrees(float RadVal)
    {
        return RadVal * (180.0f / PI);
    }

    [[nodiscard]] static FORCEINLINE constexpr double RadiansToDegrees(double RadVal)
    {
        return RadVal * (180.0 / PI_DOUBLE);
    }

    template <typename T>
    [[nodiscard]] static FORCEINLINE constexpr auto DegreesToRadians(const T& DegVal) -> decltype(DegVal* (PI / 180.0f))
    {
        return DegVal * (PI / 180.0f);
    }

    [[nodiscard]] static FORCEINLINE constexpr float DegreesToRadians(float DegVal)
    {
        return DegVal * (PI / 180.0f);
    }

    [[nodiscard]] static FORCEINLINE constexpr double DegreesToRadians(double DegVal)
    {
        return DegVal * (PI_DOUBLE / 180.0);
    }

    // Returns e^Value
    static FORCEINLINE float Exp(float Value) { return expf(Value); }
    static FORCEINLINE double Exp(double Value) { return exp(Value); }

    // Returns 2^Value
    static FORCEINLINE float Exp2(float Value) { return powf(2.f, Value); /*exp2f(Value);*/ }
    static FORCEINLINE double Exp2(double Value) { return pow(2.0, Value); /*exp2(Value);*/ }

    static FORCEINLINE float Loge(float Value) { return logf(Value); }
    static FORCEINLINE double Loge(double Value) { return log(Value); }

    static FORCEINLINE float LogX(float Base, float Value) { return Loge(Value) / Loge(Base); }
    static FORCEINLINE double LogX(double Base, double Value) { return Loge(Value) / Loge(Base); }

    // 1.0 / Loge(2) = 1.4426950f
    static FORCEINLINE float Log2(float Value) { return Loge(Value) * std::numbers::log2e_v<float>; }
    // 1.0 / Loge(2) = 1.442695040888963387
    static FORCEINLINE double Log2(double Value) { return Loge(Value) * std::numbers::log2e; }

    [[nodiscard]] static FORCEINLINE bool IsNearlyEqual(float A, float B, float ErrorTolerance = KINDA_SMALL_NUMBER)
    {
        return Abs<float>(A - B) <= ErrorTolerance;
    }

    [[nodiscard]] static FORCEINLINE bool IsNearlyEqual(double A, double B, double ErrorTolerance = SMALL_NUMBER)
    {
        return Abs<double>(A - B) <= ErrorTolerance;
    }

    [[nodiscard]] static FORCEINLINE bool IsNearlyZero(float Value, float ErrorTolerance = KINDA_SMALL_NUMBER)
    {
        return Abs<float>(Value) <= ErrorTolerance;
    }

    [[nodiscard]] static FORCEINLINE bool IsNearlyZero(double Value, double ErrorTolerance = SMALL_NUMBER)
    {
        return Abs<double>(Value) <= ErrorTolerance;
    }

    [[nodiscard]] static FORCEINLINE double Cos(double RadVal) { return cos(RadVal); }
    [[nodiscard]] static FORCEINLINE float Cos(float RadVal) { return cosf(RadVal); }

    [[nodiscard]] static FORCEINLINE double Sin(double RadVal) { return sin(RadVal); }
    [[nodiscard]] static FORCEINLINE float Sin(float RadVal) { return sinf(RadVal); }

    [[nodiscard]] static FORCEINLINE double Tan(double RadVal) { return tan(RadVal); }
    [[nodiscard]] static FORCEINLINE float Tan(float RadVal) { return tanf(RadVal); }

    [[nodiscard]] static FORCEINLINE double Acos(double Value) { return acos(Value); }
    [[nodiscard]] static FORCEINLINE float Acos(float Value) { return acosf(Value); }

    [[nodiscard]] static FORCEINLINE double Asin(double Value) { return asin(Value); }
    [[nodiscard]] static FORCEINLINE float Asin(float Value) { return asinf(Value); }

    [[nodiscard]] static FORCEINLINE double Atan(double Value) { return atan(Value); }
    [[nodiscard]] static FORCEINLINE float Atan(float Value) { return atanf(Value); }

    [[nodiscard]] static FORCEINLINE double Atan2(double Y, double X) { return atan2(Y, X); }
    [[nodiscard]] static FORCEINLINE float Atan2(float Y, float X) { return atan2f(Y, X); }

    static FORCEINLINE void SinCos(float* ScalarSin, float* ScalarCos, float Value)
    {
        *ScalarSin = sinf(Value);
        *ScalarCos = cosf(Value);
    }

    static FORCEINLINE void SinCos(double* ScalarSin, double* ScalarCos, double Value)
    {
        *ScalarSin = sin(Value);
        *ScalarCos = cos(Value);
    }

    template <typename T>
    [[nodiscard]] static FORCEINLINE T Square(T Value) { return Value * Value; }


    [[nodiscard]] static FORCEINLINE int32 CeilToInt(float Value) { return static_cast<int32>(ceilf(Value)); }
    [[nodiscard]] static FORCEINLINE int32 CeilToInt(double Value) { return static_cast<int32>(ceil(Value)); }

    template <typename T>
    [[nodiscard]] static FORCEINLINE int32 CeilToInt(T Value) { return static_cast<int32>(ceil(Value)); }


    [[nodiscard]] static FORCEINLINE float UnwindDegrees(float A)
    {
        while (A > 180.0f)
        {
            A -= 360.0f;
        }
        while (A < -180.0f)
        {
            A += 360.0f;
        }
        return A;
    }

    [[nodiscard]] static float Fmod(float X, float Y)
    {
        const float AbsY = FMath::Abs(Y);
        if (AbsY <= SMALL_NUMBER)
        {
            return 0.0;
        }

        return fmodf(X, Y);
    }

    template<typename T1, typename T2 = T1, typename T3 = T2, typename T4 = T3>
    [[nodiscard]] static auto FInterpConstantTo(T1 Current, T2 Target, T3 DeltaTime, T4 InterpSpeed)
    {
        static_assert(!std::is_same_v<T1, bool> && !std::is_same_v<T2, bool>, "Boolean types may not be interpolated");
        using RetType = decltype(T1()* T2()* T3()* T4());

        const RetType Dist = Target - Current;

        // If distance is too small, just set the desired location
        if (FMath::Square(Dist) < SMALL_NUMBER)
        {
            return static_cast<RetType>(Target);
        }

        const RetType Step = InterpSpeed * DeltaTime;
        return Current + FMath::Clamp(Dist, -Step, Step);
    }

    template<typename T1, typename T2 = T1, typename T3 = T2, typename T4 = T3>
    [[nodiscard]] static auto FInterpTo(T1  Current, T2 Target, T3 DeltaTime, T4 InterpSpeed)
    {
        static_assert(!std::is_same_v<T1, bool> && !std::is_same_v<T2, bool>, "Boolean types may not be interpolated");
        using RetType = decltype(T1()* T2()* T3()* T4());

        // If no interp speed, jump to target value
        if (InterpSpeed <= 0.f)
        {
            return static_cast<RetType>(Target);
        }

        // Distance to reach
        const RetType Dist = Target - Current;

        // If distance is too small, just set the desired location
        if (FMath::Square(Dist) < SMALL_NUMBER)
        {
            return static_cast<RetType>(Target);
        }

        // Delta Move, Clamp so we do not over shoot.
        const RetType DeltaMove = Dist * FMath::Clamp<RetType>(DeltaTime * InterpSpeed, 0.f, 1.f);

        return Current + DeltaMove;
    }
    /** Interpolate between A and B, applying an ease in function.  Exp controls the degree of the curve. */
    template< class T >
    [[nodiscard]] static FORCEINLINE T InterpEaseIn(const T& A, const T& B, float Alpha, float Exp)
    {
        float const ModifiedAlpha = Pow(Alpha, Exp);
        return Lerp<T>(A, B, ModifiedAlpha);
    }

    /** Interpolate between A and B, applying an ease out function.  Exp controls the degree of the curve. */
    template< class T >
    [[nodiscard]] static FORCEINLINE T InterpEaseOut(const T& A, const T& B, float Alpha, float Exp)
    {
        float const ModifiedAlpha = 1.f - Pow(1.f - Alpha, Exp);
        return Lerp<T>(A, B, ModifiedAlpha);
    }
    /** Interpolate between A and B, applying an ease in/out function.  Exp controls the degree of the curve. */
    template< class T >
    [[nodiscard]] static FORCEINLINE T InterpEaseInOut(const T& A, const T& B, float Alpha, float Exp)
    {
        return Lerp<T>(A, B, (Alpha < 0.5f) ?
            InterpEaseIn(0.f, 1.f, Alpha * 2.f, Exp) * 0.5f :
            InterpEaseOut(0.f, 1.f, Alpha * 2.f - 1.f, Exp) * 0.5f + 0.5f);
    }
    template<typename T>
    [[nodiscard]] static constexpr T SmoothStep(T A, T B, T X)
    {
        if (X < A)
        {
            return 0;
        }
        else if (X >= B)
        {
            return 1;
        }
        const T InterpFraction = (X - A) / (B - A);
        return InterpFraction * InterpFraction * (3.0f - 2.0f * InterpFraction);
    }

    template< class T >
    [[nodiscard]] static constexpr FORCEINLINE T LerpStable(const T& A, const T& B, double Alpha)
    {
        return (T)((A * (1.0 - Alpha)) + (B * Alpha));
    }

    /** Performs a linear interpolation between two values, Alpha ranges from 0-1. Handles full numeric range of T */
    template< class T >
    [[nodiscard]] static constexpr FORCEINLINE T LerpStable(const T& A, const T& B, float Alpha)
    {
        return (T)((A * (1.0f - Alpha)) + (B * Alpha));
    }

    static float FRand()
    {
        static std::mt19937_64 RNG{ std::random_device{}() };
        static std::uniform_real_distribution<float> Dist{ 0.0f, 1.0f };
        return Dist(RNG);
    }

    static float FRandRange(float Min, float Max)
    {
        if (Min > Max) std::swap(Min, Max);
        return Min + (Max - Min) * FRand();
    }
};
