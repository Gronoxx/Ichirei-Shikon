//
// Created by Lucas N. Ferreira on 22/05/25.
//

#pragma once

#include "UIElement.h"
#include <SDL.h>

class UIRect : public UIElement {
public:
    UIRect(const Vector2& pos, const Vector2& size, const Vector3& color);

    void Draw(SDL_Renderer* renderer, const Vector2& screenPos) override;
};
