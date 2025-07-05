//
// Created by Lucas N. Ferreira on 22/05/25.
//

#include "UIRect.h"

UIRect::UIRect(const Vector2& pos, const Vector2& size, const Vector3& color)
    : UIElement(pos, size, color)
{
}
UIRect::UIRect(const Vector2& pos, const Vector2& size, const Vector4& color)
    : UIElement(pos, size, Vector4(color.x, color.y, color.z, color.w))
{
}

void UIRect::Draw(SDL_Renderer* renderer, const Vector2& screenPos)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Define cor de desenho com base em mColor
    SDL_SetRenderDrawColor(
        renderer,
        static_cast<Uint8>(mColor.x),
        static_cast<Uint8>(mColor.y),
        static_cast<Uint8>(mColor.z),
        static_cast<Uint8>(mColor.w)
    );
    SDL_Rect rect = { static_cast<int>(screenPos.x + mPosition.x),
        static_cast<int>(screenPos.y + mPosition.y),
        static_cast<int>(mSize.x), static_cast<int>(mSize.y) };
    SDL_RenderFillRect(renderer, &rect);
}
