// UITimerBar.cpp

#include "UITimerBar.h"
#include <algorithm>

UITimerBar::UITimerBar(const Vector2& pos, const Vector2& size, float duration)
    : UIElement(pos, size, Vector3(0,0,0)),
      mDuration(duration), mTimeRemaining(duration) {
}

UITimerBar::~UITimerBar() {
}

void UITimerBar::Update(const float deltaTime)
{
    mTimeRemaining = std::max(0.0f, mTimeRemaining - deltaTime);
}

void UITimerBar::Draw(SDL_Renderer* renderer, const Vector2& offset)
{
    const float percent = mTimeRemaining / mDuration;
    const int filledWidth = static_cast<int>(mSize.x * percent);

    for (int i = 0; i < filledWidth; ++i)
    {
        const float t = static_cast<float>(i) / mSize.x;

        // Interpolação de azul (left) → rosa (right)
        const Uint8 r = static_cast<Uint8>(91 + t * (255 - 91));
        const Uint8 g = static_cast<Uint8>(187 + t * (20 - 187));
        const Uint8 b = static_cast<Uint8>(255 + t * (147 - 255));

        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        SDL_RenderDrawLine(renderer,
            static_cast<int>(mPosition.x + offset.x + i),
            static_cast<int>(mPosition.y + offset.y),
            static_cast<int>(mPosition.x + offset.x + i),
            static_cast<int>(mPosition.y + offset.y + mSize.y));
    }
}
