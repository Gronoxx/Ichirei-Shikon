//
// Created by Lucas N. Ferreira on 22/05/25.
//

#pragma once

#include <string>
#include <SDL.h>
#include "../GameMath.h"
#include "UIElement.h"

class UIText : public UIElement {
public:
    UIText(const std::string& text, class UIFont* font, int pointSize = 40, const unsigned wrapLength = 1024,
           const Vector2 &pos = Vector2::Zero, const Vector2 &size = Vector2(230.f, 22.0f), const Vector3& color = Color::White);

    ~UIText();

    void SetText(const std::string& name);
    void Draw(SDL_Renderer* renderer, const Vector2 &screenPos) override;
    SDL_Texture* GetTextTexture(){return mTextTexture;};

protected:
    std::string mText;
    class UIFont* mFont;
    SDL_Texture *mTextTexture;

    unsigned int mPointSize;
    unsigned int mWrapLength;
};
