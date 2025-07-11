#pragma once

#include "UIElement.h"
#include <SDL.h>

class UIRect final : public UIElement {
public:
    UIRect(const Vector2& pos, const Vector2& size, const Vector3& color);
    UIRect(const Vector2& pos, const Vector2& size, const Vector4& color);

    void Draw(SDL_Renderer* renderer, const Vector2& screenPos) override;
};
