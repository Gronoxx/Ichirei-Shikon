//
// Created by Lucas N. Ferreira on 28/05/25.
//

#include "UIImage.h"

UIImage::UIImage(SDL_Renderer * renderer, const std::string &imagePath, const Vector2 &pos, const Vector2 &size, const Vector3 &color)
    : UIElement(pos, size, color),
    mTexture(nullptr),
    mRenderer(renderer),
    mIsVisible(true)
{
    SDL_Surface* surface = IMG_Load(imagePath.c_str());

    if (!surface) {
        SDL_Log("Failed to load image: %s", IMG_GetError());
    }else {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(mRenderer, surface);
        SDL_FreeSurface(surface);

        if (!texture) {
            SDL_Log("Failed to create texture: %s", SDL_GetError());
        }
        else {
            mTexture = texture;
        }

    }
}

UIImage::~UIImage()
{
    // O destrutor agora não faz nada com a textura.
    // O ponteiro mTexture era apenas um "empréstimo" do TextureManager (Game).
    // O Game é o verdadeiro dono e será responsável por liberar a memória
    // da textura em Game::Shutdown().
}

void UIImage::Draw(SDL_Renderer* renderer, const Vector2 &screenPos)
{
    if (!mTexture || !mIsVisible) {
        return;
    }

    SDL_Rect src = {static_cast<int>(screenPos.x + mPosition.x),
                         static_cast<int>(screenPos.y + mPosition.y),
                         static_cast<int>(mSize.x),
                         static_cast<int>(mSize.y)};

    SDL_RenderCopyEx(renderer,mTexture,NULL,&src,0.0,NULL,SDL_FLIP_NONE);
}