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
}

DrawSpriteComponent::~DrawSpriteComponent()
{

}

void DrawSpriteComponent::Draw(SDL_Renderer *renderer, const Vector3 &modColor) {

    if (mCustomDstRect) {
        mDstRect = {
            static_cast<int>(mDstRect_aux.x - mOwner->GetGame()->GetCameraPos().x),
            static_cast<int>(mDstRect_aux.y - mOwner->GetGame()->GetCameraPos().y),
            static_cast<int>(mDstRect_aux.w * mOwner->GetScale()),
            static_cast<int> (mDstRect_aux.h * mOwner->GetScale())
        };
    }
    else  {
        mDstRect = {
            static_cast<int>(mOwner->GetPosition().x - mOwner->GetGame()->GetCameraPos().x),
            static_cast<int>(mOwner->GetPosition().y - mOwner->GetGame()->GetCameraPos().y),
            static_cast<int>(mWidth * mOwner->GetScale()),
            static_cast<int> (mHeight * mOwner->GetScale())
        };
    }



    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (mOwner->GetRotation() == Math::Pi) {
        flip = SDL_FLIP_HORIZONTAL;
    }

    SDL_SetTextureBlendMode(mSpriteSheetSurface, SDL_BLENDMODE_BLEND);

    SDL_SetTextureColorMod(mSpriteSheetSurface,
                           static_cast<Uint8>(modColor.x),
                           static_cast<Uint8>(modColor.y),
                           static_cast<Uint8>(modColor.z));

    if (mCustomSourceRect) {
        SDL_RenderCopyEx(renderer, mSpriteSheetSurface, &mSourceRect, &mDstRect, mOwner->GetRotation(), nullptr, flip);
    }else{
        SDL_RenderCopyEx(renderer, mSpriteSheetSurface, nullptr, &mDstRect, mOwner->GetRotation(), nullptr, flip);
    }
}

void DrawSpriteComponent::SetTexture(SDL_Texture* texture,const std::string &texturePath) {
    mSpriteSheetSurface = texture;
    mTexturePath = texturePath;
}