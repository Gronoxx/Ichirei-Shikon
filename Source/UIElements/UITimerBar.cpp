// UITimerBar.cpp

#include "UITimerBar.h"
#include <algorithm>

UITimerBar::UITimerBar(const Vector2& pos, const Vector2& size, float duration)
    : UIElement(pos, size, Vector3(0,0,0)), // cor é ignorada aqui
      mDuration(duration), mTimeRemaining(duration) {
    //SDL_Log("UITimerBar created: duration = %f", mDuration);
}

UITimerBar::~UITimerBar() {
    // SDL_Log("UITimerBar destroyed");
}

void UITimerBar::Update(float deltaTime)
{
    mTimeRemaining = std::max(0.0f, mTimeRemaining - deltaTime);
}

void UITimerBar::Draw(SDL_Renderer* renderer, const Vector2& offset)
{
    float percent = mTimeRemaining / mDuration;
    int filledWidth = static_cast<int>(mSize.x * percent);

    for (int i = 0; i < filledWidth; ++i)
    {
        float t = static_cast<float>(i) / mSize.x;

        // Interpolação de azul (left) → rosa (right)
        Uint8 r = static_cast<Uint8>(91 + t * (255 - 91));
        Uint8 g = static_cast<Uint8>(187 + t * (20 - 187));
        Uint8 b = static_cast<Uint8>(255 + t * (147 - 255));

        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderDrawLine(renderer,
            static_cast<int>(mPosition.x + offset.x + i),
            static_cast<int>(mPosition.y + offset.y),
            static_cast<int>(mPosition.x + offset.x + i),
            static_cast<int>(mPosition.y + offset.y + mSize.y));
    }
}
