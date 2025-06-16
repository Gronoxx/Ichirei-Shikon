// DrawAnimatedComponent.cpp
#include "DrawAnimatedComponent.h"
#include "../../Actors/Actor.h"
#include "../../Game.h"
#include "../../Json.h"
#include <fstream>
#include <iostream>

DrawAnimatedComponent::DrawAnimatedComponent(Actor* owner,
                                             const std::string& defaultTexturePath,
                                             const std::string& defaultDataPath,
                                             const std::string animName,
                                             const std::vector<int>& images,
                                             int drawOrder)
    : DrawSpriteComponent(owner, defaultTexturePath, 0, 0, drawOrder),
      mAnimName(animName)
{
    LoadSpriteSheetForAnimation(animName, defaultTexturePath, defaultDataPath);
    AddAnimation(animName,images);
    SetAnimFPS(14.0f);
    SetAnimation(animName);
}

//Legacy
DrawAnimatedComponent::DrawAnimatedComponent(Actor* owner,
                                             const std::string& spriteSheetPath,
                                             const std::string& spriteSheetData,
                                             int drawOrder)
    : DrawSpriteComponent(owner, spriteSheetPath, 0, 0, drawOrder),
      mIsSingleSheet(true) // Ativa o modo de folha única
{
    // Carrega a textura e os dados usando a chave interna padrão
    LoadSpriteSheetForAnimation(SINGLE_SHEET_KEY, spriteSheetPath, spriteSheetData);

    // Define um FPS padrão, pode ser ajustado depois
    SetAnimFPS(14.0f);

    // Nenhuma animação é definida por padrão, precisa ser adicionada e definida manualmente
    mAnimName = "";
    mAnimTimer = 0.0f;
}

DrawAnimatedComponent::~DrawAnimatedComponent()
{
    for (auto& [_, frames] : mSpriteSheetDatas) {
        for (auto rect : frames) {
            delete rect;
        }
    }

    // Destrói texturas carregadas
    for (auto& [animName, texture] : mSpriteSheetTextures) {
        if (texture) {
            SDL_DestroyTexture(texture);
        }
    }


    mSpriteSheetTextures.clear();
}

void DrawAnimatedComponent::LoadSpriteSheetForAnimation(const std::string& animName,
                                                        const std::string& texturePath,
                                                        const std::string& dataPath)
{
    //Desaloca se estiver sobreescrevendo
    auto it = mSpriteSheetDatas.find(animName);
    if (it != mSpriteSheetDatas.end()) {
        for (auto rect : it->second) {
            delete rect;
        }
    }

    SDL_Texture* texture = mOwner->GetGame()->LoadTexture(texturePath);
    if (!texture) {
        SDL_Log("Erro ao carregar textura: %s", texturePath.c_str());
        return;
    }
    mSpriteSheetTextures[animName] = texture;

    // Load sprite sheet data
    std::ifstream spriteSheetFile(dataPath);
    nlohmann::json spriteSheetData = nlohmann::json::parse(spriteSheetFile);

    SDL_Rect* rect = nullptr;
    std::vector<SDL_Rect *> SpriteSheetData;

    for(const auto& frame : spriteSheetData["frames"]) {

        int x = frame["frame"]["x"].get<int>();
        int y = frame["frame"]["y"].get<int>();
        int w = frame["frame"]["w"].get<int>();
        int h = frame["frame"]["h"].get<int>();
        rect = new SDL_Rect({x, y, w, h});

        SpriteSheetData.emplace_back(rect);
    }
    mSpriteSheetDatas[animName] = SpriteSheetData;
}

void DrawAnimatedComponent::Draw(SDL_Renderer* renderer, const Vector3& modColor)
{
    auto animIt = mAnimations.find(mAnimName);
    if (animIt == mAnimations.end()) {
        // Animação atual não existe
        return;
    }

    const auto& frameOrder = animIt->second;
    if (frameOrder.empty()) {
        return;
    }

    int currentFrame = static_cast<int>(mAnimTimer);
    if (currentFrame >= (int)frameOrder.size()) {
        currentFrame = (int)frameOrder.size() - 1;
    }

    int spriteIdx = frameOrder[currentFrame];

    //Legacy
    std::string sheetKey = mIsSingleSheet ? SINGLE_SHEET_KEY : mAnimName;

    auto texIt = mSpriteSheetTextures.find(sheetKey);
    auto framesIt = mSpriteSheetDatas.find(sheetKey);

    if (texIt == mSpriteSheetTextures.end() || framesIt == mSpriteSheetDatas.end()) {
        SDL_Log("Erro: textura ou frames ausentes para a chave de folha '%s'", sheetKey.c_str());
        return;
    }

    SDL_Texture* texture = texIt->second;
    const auto& frames = framesIt->second;

    if (spriteIdx < 0 || spriteIdx >= (int)frames.size()) {
        return; // índice inválido
    }

    const SDL_Rect* srcRect = frames[spriteIdx];

    Vector2 currentOffset(0, 0);
    auto offsetIt = mAnimFrameOffsets.find(mAnimName);
    if (offsetIt != mAnimFrameOffsets.end()) {
        const std::vector<Vector2>& offsets = offsetIt->second;
        if (spriteIdx >= 0 && spriteIdx < (int)offsets.size()) {
            currentOffset = offsets[spriteIdx];
        }
    }

    float scale = mOwner->GetScale();

    SDL_Rect dstRect = {
        static_cast<int>(mOwner->GetPosition().x - mOwner->GetGame()->GetCameraPos().x - currentOffset.x * scale),
        static_cast<int>(mOwner->GetPosition().y - mOwner->GetGame()->GetCameraPos().y - currentOffset.y * scale),
        static_cast<int>(srcRect->w * scale),
        static_cast<int>(srcRect->h * scale)
    };

    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (mOwner->GetRotation() == Math::Pi) {
        flip = SDL_FLIP_HORIZONTAL;
    }

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureColorMod(texture,
                           static_cast<Uint8>(modColor.x),
                           static_cast<Uint8>(modColor.y),
                           static_cast<Uint8>(modColor.z));

    SDL_RenderCopyEx(renderer, texture, srcRect, &dstRect, mOwner->GetRotation(), nullptr, flip);
}

void DrawAnimatedComponent::Update(float deltaTime)
{
    if (mIsPaused) {
        return;
    }

    mAnimTimer += mAnimFPS * deltaTime;

    auto animIt = mAnimations.find(mAnimName);
    if (animIt == mAnimations.end()) {
        return;
    }

    int frameCount = static_cast<int>(animIt->second.size());
    if (frameCount > 0) {
        mAnimTimer = fmodf(mAnimTimer, static_cast<float>(frameCount));
    }
}

void DrawAnimatedComponent::SetAnimation(const std::string& name)
{
    if (mAnimName == name) return; // evita resetar animação igual
    if (mAnimations.find(name) == mAnimations.end()) {
        SDL_Log("Aviso: animação '%s' não encontrada!", name.c_str());
        return;
    }

    mAnimName = name;
    mAnimTimer = 0.0f; // reseta o timer da animação
}

void DrawAnimatedComponent::AddAnimation(const std::string& name, const std::vector<int>& spriteNums)
{
    if (mAnimations.find(name) != mAnimations.end()) {
        SDL_Log("Aviso: animação '%s' já existente. Substituindo.", name.c_str());
    }
    mAnimations[name] = spriteNums;
}

void DrawAnimatedComponent::AddAnimationOffsets(const std::string& animName, const std::vector<Vector2>& offsets)
{
    mAnimFrameOffsets[animName] = NormalizeOffsets(offsets);
}

std::vector<Vector2> DrawAnimatedComponent::NormalizeOffsets(const std::vector<Vector2>& originalOffsets)
{
    std::vector<Vector2> normalizedOffsets;
    if (originalOffsets.empty()) {
        return normalizedOffsets;
    }

    Vector2 reference = originalOffsets[0];

    for (const Vector2& offset : originalOffsets) {
        normalizedOffsets.push_back(offset - reference);
    }
    return normalizedOffsets;
}
