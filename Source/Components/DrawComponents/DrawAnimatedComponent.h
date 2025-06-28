#pragma once
#include "DrawSpriteComponent.h"
#include <string>
#include <vector>
#include <map>

class DrawAnimatedComponent : public DrawSpriteComponent
{
public:
    // Construtor simplificado
    DrawAnimatedComponent(class Actor* owner, int drawOrder = 100);
    DrawAnimatedComponent(Actor* owner,
                                             const std::string& defaultTexturePath,
                                             const std::string& defaultDataPath,
                                             const std::string animName,
                                             const std::vector<int>& images,
                                             int drawOrder);
    ~DrawAnimatedComponent();

    // Sobrescreve Draw e Update
    void Draw(SDL_Renderer* renderer, const Vector3& modColor = Color::White) override;
    void Update(float deltaTime) override;
    
    void LoadCharacterAnimations(const std::string& characterJsonPath);

    // Funções de controle
    void SetAnimation(const std::string& name);
    const std::string& GetAnimation() const { return mCurrentAnimationName; }
    void SetIsPaused(bool paused) { mIsPaused = paused; }
    void SetRenderOffset(const Vector2& offset) { mRenderOffset = offset; }
    bool IsAnimationFinished() const;


private:
    // Carrega a textura e os dados do spritesheet para UMA animação.
    void LoadSpriteSheetForAnimation(const std::string& animName, const std::string& texturePath, const std::string& dataPath);

    // Armazena a ordem dos frames para cada animação. Ex: "run" -> {0, 1, 2, 3...}
    std::map<std::string, std::vector<int>> mAnimationFrames;

    // Armazena o FPS para cada animação. Ex: "run" -> 24.0
    std::map<std::string, float> mAnimationFPS;

    // Armazena as texturas de cada animação (um spritesheet por animação).
    std::map<std::string, SDL_Texture*> mSpriteSheetTextures;

    // Armazena os retângulos de cada frame de cada spritesheet.
    std::map<std::string, std::vector<SDL_Rect*>> mSpriteSheetData;

    // Estado da animação atual
    std::string mCurrentAnimationName;
    float mCurrentFrame;
    bool mIsPaused;

    //Offsets
    Vector2 mRenderOffset;

    std::map<std::string, std::vector<std::string>> mFrameTexturePaths;


};