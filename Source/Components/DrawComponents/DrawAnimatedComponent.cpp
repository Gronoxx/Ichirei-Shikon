#include "DrawAnimatedComponent.h"
#include "../../Actors/Actor.h"
#include "../../Game.h"
#include "../../Json.h" // nlohmann::json
#include <fstream>
#include <iostream>


// Construtor simplificado. Não faz quase nada.
DrawAnimatedComponent::DrawAnimatedComponent(Actor* owner, int drawOrder)
    : DrawSpriteComponent(owner, "", 0, 0, drawOrder),
      mCurrentFrame(0.0f),
      mIsPaused(false),
      mRenderOffset(0.0f, 0.0f)
{
}

DrawAnimatedComponent::~DrawAnimatedComponent()
{
    // Limpa os retângulos alocados dinamicamente
    for (auto& pair : mSpriteSheetData) {
        for (auto rect : pair.second) {
            delete rect;
        }
    }
    // As texturas são gerenciadas pelo Game::mTextures, não precisa destruir aqui
    // se você seguir o padrão do livro. Se não, descomente:
    // for (auto& pair : mSpriteSheetTextures) {
    //     SDL_DestroyTexture(pair.second);
    // }
}

// A NOVA FUNÇÃO PRINCIPAL
void DrawAnimatedComponent::LoadCharacterAnimations(const std::string& characterJsonPath)
{
    std::ifstream file(characterJsonPath);
    if (!file.is_open()) {
        SDL_Log("ERRO: Não foi possível abrir o manifesto de animação: %s", characterJsonPath.c_str());
        return;
    }

    // 1. Parseia o arquivo JSON inteiro
    nlohmann::json data = nlohmann::json::parse(file);

    // 2. Carrega o offset de renderização global, se existir
    if (data.contains("renderOffset"))
    {
        Vector2 offset(
            data["renderOffset"]["x"].get<float>(),
            data["renderOffset"]["y"].get<float>()
        );
        // Chama a função private ou atribui diretamente ao membro
        SetRenderOffset(offset);
        SDL_Log("Render offset loaded: (%.2f, %.2f)", mRenderOffset.x, mRenderOffset.y);
    }
    else
    {
        SDL_Log("AVISO: 'renderOffset' não encontrado no manifesto %s", characterJsonPath.c_str());
    }

    // 3. Carrega o bloco de animações, se existir
    if (data.contains("animations"))
    {
        const auto& animations = data["animations"];
        for (auto& [animName, animData] : animations.items()) {
            std::string texturePath = animData["texturePath"];
            std::string dataPath = animData["dataPath"];
            float fps = animData["fps"];
            std::vector<int> frameOrder = animData["frameOrder"];

            LoadSpriteSheetForAnimation(animName, texturePath, dataPath);
            mAnimationFrames[animName] = frameOrder;
            mAnimationFPS[animName] = fps;

            SDL_Log("Animação '%s' carregada com %zu frames e FPS %.1f", animName.c_str(), frameOrder.size(), fps);
        }
    }
}

// Função auxiliar para carregar um único spritesheet
void DrawAnimatedComponent::LoadSpriteSheetForAnimation(const std::string& animName, const std::string& texturePath, const std::string& dataPath)
{
    // Carrega a textura
    mSpriteSheetTextures[animName] = mOwner->GetGame()->LoadTexture(texturePath);

    // Carrega os dados do JSON do spritesheet
    std::ifstream file(dataPath);
    nlohmann::json sheetData = nlohmann::json::parse(file);

    std::vector<SDL_Rect*> frames;
    for (const auto& frame : sheetData["frames"]) {
        frames.push_back(new SDL_Rect{
            frame["frame"]["x"].get<int>(),
            frame["frame"]["y"].get<int>(),
            frame["frame"]["w"].get<int>(),
            frame["frame"]["h"].get<int>()
        });
    }
    mSpriteSheetData[animName] = frames;
}


void DrawAnimatedComponent::Update(float deltaTime)
{
    if (mIsPaused || mCurrentAnimationName.empty()) {
        return;
    }

    // Acha os dados da animação atual
    auto animIt = mAnimationFrames.find(mCurrentAnimationName);
    if (animIt == mAnimationFrames.end()) {
        return; // Animação não encontrada, não faz nada
    }

    // Pega o FPS para a animação atual
    float fps = mAnimationFPS.at(mCurrentAnimationName);

    // Atualiza o contador de frame
    mCurrentFrame += fps * deltaTime;

    // Faz o loop da animação
    int frameCount = static_cast<int>(animIt->second.size());
    if (frameCount > 0) {
        while (mCurrentFrame >= frameCount) {
            mCurrentFrame -= frameCount;
        }
    }
}

void DrawAnimatedComponent::Draw(SDL_Renderer* renderer, const Vector3& modColor)
{
    if (mCurrentAnimationName.empty()) {
        return;
    }

    // Acha os dados da animação atual
    auto& frameOrder = mAnimationFrames.at(mCurrentAnimationName);
    auto& sheetFrames = mSpriteSheetData.at(mCurrentAnimationName);
    auto& texture = mSpriteSheetTextures.at(mCurrentAnimationName);

    if (frameOrder.empty() || !texture) {
        return;
    }

    // Pega o índice do frame atual na ordem da animação
    int frameIdx = static_cast<int>(mCurrentFrame);
    int spriteIdx = frameOrder[frameIdx];

    const SDL_Rect* srcRect = sheetFrames[spriteIdx];

    float scale = mOwner->GetScale();
    SDL_Rect dstRect = {
        static_cast<int>(mOwner->GetPosition().x - mOwner->GetGame()->GetCameraPos().x - (mRenderOffset.x * scale) - (Game::TILE_SIZE * scale - Game::TILE_SIZE)),
        static_cast<int>(mOwner->GetPosition().y - mOwner->GetGame()->GetCameraPos().y - (mRenderOffset.y * scale) - (Game::TILE_SIZE * scale - Game::TILE_SIZE)),
        static_cast<int>(srcRect->w * scale),
        static_cast<int>(srcRect->h * scale)
    };

    SDL_RendererFlip flip = (mOwner->GetRotation() == Math::Pi) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureColorMod(texture,
                           static_cast<Uint8>(modColor.x),
                           static_cast<Uint8>(modColor.y),
                           static_cast<Uint8>(modColor.z));

    SDL_RenderCopyEx(renderer, texture, srcRect, &dstRect, 0.0, nullptr, flip); // Rotação é tratada pelo flip
}

void DrawAnimatedComponent::SetAnimation(const std::string& name)
{
    // Verifica se a animação existe
    if (mAnimationFrames.find(name) == mAnimationFrames.end()) {
        SDL_Log("AVISO: Tentativa de definir animação inexistente: '%s'", name.c_str());
        return;
    }

    // Se não for a mesma, reseta o timer
    if (mCurrentAnimationName != name) {
        mCurrentAnimationName = name;
        mCurrentFrame = 0.0f;
    }
}






