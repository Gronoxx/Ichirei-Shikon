//
// Created by Lucas N. Ferreira on 28/09/23.
//

#include "DrawSpriteComponent.h"
#include "../../Actors/Actor.h"
#include "../../Game.h"

DrawSpriteComponent::DrawSpriteComponent(class Actor* owner, const std::string &texturePath, const int width, const int height, const int drawOrder)
        :DrawComponent(owner, drawOrder)
        ,mWidth(width)
        ,mHeight(height)
{
    mTexturePath = texturePath;
    mSpriteSheetSurface = mOwner->GetGame()->LoadTexture(texturePath);
    mCustomDstRect = false;
    mCustomSourceRect = false;
}

DrawSpriteComponent::~DrawSpriteComponent()
{

}

void DrawSpriteComponent::Draw(SDL_Renderer *renderer, const Vector3 &modColor) {

    // Pega o fator de zoom da câmera
    Vector2 cameraPos = mOwner->GetGame()->GetCameraPos();

    if (mCustomDstRect) {
        // Lógica para retângulo de destino customizado (se você usar)
        mDstRect = {
            static_cast<int>((mDstRect_aux.x - cameraPos.x)),
            static_cast<int>((mDstRect_aux.y - cameraPos.y)),
            static_cast<int>(mDstRect_aux.w * mOwner->GetScale()),
            static_cast<int>(mDstRect_aux.h * mOwner->GetScale())
        };
    }
    else  {
        // Lógica principal de desenho
        mDstRect = {
            // A posição do ator na tela é (pos_ator - pos_camera) * zoom
            static_cast<int>((mOwner->GetPosition().x - cameraPos.x) ),
            static_cast<int>((mOwner->GetPosition().y - cameraPos.y) ),
            // A largura do ator na tela é largura_original * escala_ator * zoom
            static_cast<int>(mWidth * mOwner->GetScale() ),
            static_cast<int>(mHeight * mOwner->GetScale() )
        };
    }

    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (mOwner->GetRotation() >= Math::Pi) {
        flip = SDL_FLIP_HORIZONTAL;
    }

    SDL_SetTextureBlendMode(mSpriteSheetSurface, SDL_BLENDMODE_BLEND);
    SDL_SetTextureColorMod(mSpriteSheetSurface,
                           static_cast<Uint8>(modColor.x),
                           static_cast<Uint8>(modColor.y),
                           static_cast<Uint8>(modColor.z));

    if (mCustomSourceRect) {
        SDL_RenderCopyEx(renderer, mSpriteSheetSurface, &mSourceRect, &mDstRect, mOwner->GetRotation(), nullptr, flip);
    } else {
        SDL_RenderCopyEx(renderer, mSpriteSheetSurface, nullptr, &mDstRect, mOwner->GetRotation(), nullptr, flip);
    }
}

void DrawSpriteComponent::SetTexture(SDL_Texture* texture,const std::string &texturePath) {
    mSpriteSheetSurface = texture;
    mTexturePath = texturePath;
}