//
// Created by Lucas N. Ferreira on 28/09/23.
//

#pragma once

#include "DrawSpriteComponent.h"
#include <unordered_map>

class DrawAnimatedComponent : public DrawSpriteComponent {
public:
    // (Lower draw order corresponds with further back)
    //New Constructor
    DrawAnimatedComponent(class Actor* owner, const std::string &spriteSheetPath, const std::string &spriteSheetData, const std::string animName,const std::vector<int>& images, int drawOrder = 100);
    //Old Version (Legacy)
    DrawAnimatedComponent(class Actor* owner,
                          const std::string& spriteSheetPath,
                          const std::string& spriteSheetData,
                          int drawOrder = 100);
    ~DrawAnimatedComponent() override;

    void Draw(SDL_Renderer* renderer, const Vector3 &modColor = Color::White) override;
    void Update(float deltaTime) override;

    // Use to change the FPS of the animation
    void SetAnimFPS(float fps) { mAnimFPS = fps; }

    // Set the current active animation
    void SetAnimation(const std::string& name);

    // Use to pause/unpause the animation
    void SetIsPaused(bool pause) { mIsPaused = pause; }

    // Add an animation of the corresponding name to the animation map
    void AddAnimation(const std::string& name, const std::vector<int>& images);

    // Offsets
    std::vector<Vector2> NormalizeOffsets(const std::vector<Vector2>& originalOffsets);
    void AddAnimationOffsets(const std::string& animName, const std::vector<Vector2>& offsets);
    void LoadSpriteSheetForAnimation(const std::string& animName, const std::string& texturePath, const std::string& dataPath);

private:

    // Mapa animação -> textura (spritesheet)
    std::unordered_map<std::string, SDL_Texture*> mSpriteSheetTextures;

    // Mapa animação -> Datas (SDL_Rect)
    std::unordered_map<std::string, std::vector<SDL_Rect *>> mSpriteSheetDatas;

    // Mapa animação -> sequência de índices dos frames (ordem da animação)
    std::unordered_map<std::string, std::vector<int>> mAnimations;

    // Name of current animation
    std::string mAnimName;

    // Tracks current elapsed time in animation
    float mAnimTimer = 0.0f;

    // The frames per second the animation should run at
    float mAnimFPS = 10.0f;

    // Whether or not the animation is paused (defaults to false)
    bool mIsPaused = false;

    //Offsets
    std::unordered_map<std::string, std::vector<Vector2>> mAnimFrameOffsets;

    //Legacy
    bool mIsSingleSheet = false; // Flag para indicar o modo de operação
    const std::string SINGLE_SHEET_KEY = "default_sheet"; // Chave interna para a folha única
};
