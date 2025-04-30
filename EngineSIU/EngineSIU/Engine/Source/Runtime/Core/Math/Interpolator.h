#pragma once
// 공통 인터페이스
struct IInterpolator
{
    virtual ~IInterpolator() = default;
    virtual float Evaluate(float t) const = 0;
};

// 1) 선형 보간
struct LinearInterp : IInterpolator
{
    float Evaluate(float t) const override
    {
        return t;
    }
};

// 2) 이징 인(Quadratic)
struct EaseInInterp : IInterpolator
{
    float Evaluate(float t) const override
    {
        return t * t;
    }
};

// 3) 이징 아웃(Quadratic)
struct EaseOutInterp : IInterpolator
{
    float Evaluate(float t) const override
    {
        return t * (2.f - t);
    }
};

// 4) Smoothstep (3차 Hermite)
struct SmoothStepInterp : IInterpolator
{
    float Evaluate(float t) const override
    {
        return t * t * (3.f - 2.f * t);
    }
};
