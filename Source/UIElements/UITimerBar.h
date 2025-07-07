// UITimerBar.h
#pragma once
#include "UIElement.h"

class UITimerBar final : public UIElement {
public:
    UITimerBar(const Vector2& pos, const Vector2& size, float duration);
    ~UITimerBar() override;

    void Update(float deltaTime);
    void Draw(SDL_Renderer* renderer, const Vector2& offset) override;

private:
    float mDuration;
    float mTimeRemaining;
};
