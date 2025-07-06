//
// Created by Lucas N. Ferreira on 28/09/23.
//

#pragma once
#include "DrawComponent.h"
#include <string>

class DrawSpriteComponent : public DrawComponent
{
public:
    // (Lower draw order corresponds with further back)
    DrawSpriteComponent(class Actor* owner, const std::string &texturePath, int width = 0, int height = 0, int drawOrder = 100);
    ~DrawSpriteComponent() override;

    void Draw(SDL_Renderer* renderer, const Vector3 &modColor = Color::White) override;
    void SetTexture(SDL_Texture* texture, const std::string &texturePath);
    void SetSourceRect(int x, int y, int w, int h) { mSourceRect = {x, y, w, h}; }
    void EnableSourceRect(bool enable) { mCustomSourceRect = enable; }
    void SetDstRect(int x, int y, int w, int h) { mDstRect_aux = {x, y, w, h}; mCustomDstRect = true; }
    void ResetCustomSourceRect() { mSourceRect = {}; mCustomSourceRect = false; }
    void ResetCustomDstRect() { mDstRect = {}; mCustomDstRect = false; }
    std::string GetTexturePath() { return mTexturePath; }

protected:
    // Map of textures loaded
    SDL_Texture* mSpriteSheetSurface;
    std::string mTexturePath;
    SDL_Rect mSourceRect = {};
    SDL_Rect mDstRect = {};

    SDL_Rect mDstRect_aux = {};

    bool mCustomDstRect;
    bool mCustomSourceRect;

    int mWidth;
    int mHeight;
};