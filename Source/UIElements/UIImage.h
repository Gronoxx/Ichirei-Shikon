//
// Created by Lucas N. Ferreira on 28/05/25.
//

#pragma once

#include <string>
#include <SDL_image.h>
#include "UIElement.h"

class UIImage :  public UIElement
{
public:
    UIImage(SDL_Renderer* renderer,const std::string &imagePath, const Vector2 &pos = Vector2::Zero,
            const Vector2 &size = Vector2(100.f, 100.f), const Vector3 &color = Color::White);

    ~UIImage();

    void Draw(SDL_Renderer* renderer, const Vector2 &screenPos) override;
    bool GetVisible(){return mIsVisible;};
    void SetVisible(bool visible){mIsVisible = visible;};

private:
    SDL_Texture* mTexture; // Texture for the image
    SDL_Renderer* mRenderer; // Renderer
    bool mIsVisible;
};
