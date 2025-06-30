//
// Created by Lucas N. Ferreira on 22/05/25.
//

#include "UIRect.h"

UIRect::UIRect(const Vector2& pos, const Vector2& size, const Vector3& color)
    : UIElement(pos, size, color)
{
}

void UIRect::Draw(SDL_Renderer* renderer, const Vector2& screenPos)
{
    // Define cor de desenho com base em mColor
    SDL_SetRenderDrawColor(
        renderer,
        static_cast<Uint8>(mColor.x),
        static_cast<Uint8>(mColor.y),
        static_cast<Uint8>(mColor.z),
        255
    );

    SDL_Rect rect;
    rect.x = static_cast<int>(screenPos.x + mPosition.x);
    rect.y = static_cast<int>(screenPos.y + mPosition.y);
    rect.w = static_cast<int>(mSize.x);
    rect.h = static_cast<int>(mSize.y);

    SDL_RenderFillRect(renderer, &rect);
}
