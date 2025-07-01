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
        //SDL_Log("Render offset loaded: (%.2f, %.2f)", mRenderOffset.x, mRenderOffset.y);
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
            float fps = animData["fps"];

            Vector2 animOffset(0.0f, 0.0f);
            if (animData.contains("offset")) {
                animOffset.x = animData["offset"]["x"].get<float>();
                animOffset.y = animData["offset"]["y"].get<float>();
                //SDL_Log("   ↪ Offset customizado (%.1f, %.1f) para animação '%s'", animOffset.x, animOffset.y, animName.c_str());
            }
            mAnimationOffsets[animName] = animOffset;

            // Caso 1: spritesheet + data
            if (animData.contains("texturePath") && animData.contains("dataPath") && animData.contains("frameOrder")) {
                std::string texturePath = animData["texturePath"];
                std::string dataPath = animData["dataPath"];
                std::vector<int> frameOrder = animData["frameOrder"];

                LoadSpriteSheetForAnimation(animName, texturePath, dataPath);
                mAnimationFrames[animName] = frameOrder;
                mAnimationFPS[animName] = fps;

                //SDL_Log("Animação '%s' (spritesheet) carregada com %zu frames", animName.c_str(), frameOrder.size());
            }

            // Caso 2: imagens soltas por frame
            else if (animData.contains("frames")) {
                std::vector<std::string> framePaths = animData["frames"];
                mFrameTexturePaths[animName] = framePaths;
                std::vector<SDL_Texture*> textures;

                for (const std::string& path : framePaths) {
                    SDL_Texture* tex = mOwner->GetGame()->LoadTexture(path);
                    if (tex) {
                        textures.push_back(tex);
                    } else {
                        SDL_Log("ERRO: Falha ao carregar frame '%s' da animação '%s'", path.c_str(), animName.c_str());
                    }
                }


                std::vector<SDL_Rect*> frames;
                for (SDL_Texture* tex : textures) {
                    // Pegamos o tamanho da textura inteira
                    int w, h;
                    SDL_QueryTexture(tex, nullptr, nullptr, &w, &h);
                    frames.push_back(new SDL_Rect{ 0, 0, w, h });
                }

                // Podemos usar a primeira textura só para referência (opcional)
                if (!textures.empty()) {
                    mSpriteSheetTextures[animName] = textures[0]; // Usa o primeiro frame como textura principal
                }

                mSpriteSheetData[animName] = frames;
                mAnimationFrames[animName] = std::vector<int>(frames.size());
                for (int i = 0; i < frames.size(); ++i) {
                    mAnimationFrames[animName][i] = i;
                }
                mAnimationFPS[animName] = fps;
            }
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
        if (mShouldLoop) {
            while (mCurrentFrame >= frameCount) {
                mCurrentFrame -= frameCount;
            }
        } else {
            if (mCurrentFrame >= frameCount) {
                mCurrentFrame = static_cast<float>(frameCount); // Fixa além do último índice
                mIsPaused = true; // Opcional: trava a animação
            }
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

    int frameIdx = static_cast<int>(mCurrentFrame);

    // Proteção extra: impede acesso fora do vetor
    if (frameIdx < 0 || frameIdx >= static_cast<int>(frameOrder.size())) {
        return;
    }
    int spriteIdx = frameOrder[frameIdx];

    // Verifica se é uma animação com múltiplos arquivos (rects todos 0,0,w,h)
    SDL_Texture* texture = nullptr;

    bool isFrameSequence =
        sheetFrames[spriteIdx]->x == 0 &&
        sheetFrames[spriteIdx]->y == 0 &&
        mFrameTexturePaths.count(mCurrentAnimationName) > 0;

    if (isFrameSequence) {
        const auto& framePaths = mFrameTexturePaths.at(mCurrentAnimationName);
        if (spriteIdx < static_cast<int>(framePaths.size())) {
            const std::string& framePath = framePaths[spriteIdx];
            texture = mOwner->GetGame()->LoadTexture(framePath);
        } else {
            SDL_Log("⚠️ spriteIdx fora do intervalo em framePaths para animação '%s'", mCurrentAnimationName.c_str());
            return;
        }
    } else {
        texture = mSpriteSheetTextures.at(mCurrentAnimationName);
    }

    const SDL_Rect* srcRect = sheetFrames[spriteIdx];

    float scale = mOwner->GetScale();

    Vector2 offset = mRenderOffset;

    // Soma com offset específico da animação (se existir)
    auto itOffset = mAnimationOffsets.find(mCurrentAnimationName);
    if (itOffset != mAnimationOffsets.end()) {
        offset += itOffset->second;
    }

    SDL_Rect dstRect = {
        static_cast<int>(mOwner->GetPosition().x - mOwner->GetGame()->GetCameraPos().x - (offset.x * scale) - (Game::TILE_SIZE * scale - Game::TILE_SIZE)),
        static_cast<int>(mOwner->GetPosition().y - mOwner->GetGame()->GetCameraPos().y - (offset.y * scale) - (Game::TILE_SIZE * scale - Game::TILE_SIZE)),
        static_cast<int>(srcRect->w * scale),
        static_cast<int>(srcRect->h * scale)
    };

    SDL_RendererFlip flip = (mOwner->GetRotation() == Math::Pi) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureColorMod(texture,
                           static_cast<Uint8>(modColor.x),
                           static_cast<Uint8>(modColor.y),
                           static_cast<Uint8>(modColor.z));

    AABBColliderComponent* collider = mOwner->GetComponent<AABBColliderComponent>();
    if (collider && collider->IsEnabled()) {
        Vector2 min = collider->GetMin();
        Vector2 max = collider->GetMax();

        SDL_Rect rect;
        rect.x = static_cast<int>(min.x - mOwner->GetGame()->GetCameraPos().x);
        rect.y = static_cast<int>(min.y - mOwner->GetGame()->GetCameraPos().y);
        rect.w = static_cast<int>(max.x - min.x);
        rect.h = static_cast<int>(max.y - min.y);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Vermelho
        //SDL_RenderDrawRect(renderer, &rect);
    }

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

    mIsPaused = false; // ✅ DESPAUSA a animação ao trocar
}

bool DrawAnimatedComponent::IsAnimationFinished() const
{
    // Só faz sentido verificar término se a animação atual está setada e não está em loop
    if (mShouldLoop || mCurrentAnimationName.empty())
        return false;

    auto it = mAnimationFrames.find(mCurrentAnimationName);
    if (it == mAnimationFrames.end())
        return true; // Considera "terminado" se não encontrou a animação

    int frameCount = static_cast<int>(it->second.size());

    // A animação é considerada terminada se passou do último frame
    return mCurrentFrame >= static_cast<float>(frameCount);
}

float DrawAnimatedComponent::GetCurrentAnimationDuration() const {
    auto it = mAnimationFrames.find(mCurrentAnimationName);
    if (it == mAnimationFrames.end()) return 0.0f;

    int frameCount = static_cast<int>(it->second.size());
    float fps = mAnimationFPS.at(mCurrentAnimationName);
    return frameCount / fps;
}

float DrawAnimatedComponent::GetAnimationDuration(const std::string& animName) const
{
    auto itFrames = mAnimationFrames.find(animName);
    auto itFPS = mAnimationFPS.find(animName);

    if (itFrames == mAnimationFrames.end() || itFPS == mAnimationFPS.end())
        return 0.0f;

    int frameCount = static_cast<int>(itFrames->second.size());
    float fps = itFPS->second;

    return frameCount / fps;
}







