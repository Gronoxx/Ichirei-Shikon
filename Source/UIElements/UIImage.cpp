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
    // --------------
    // TODO - PARTE 1-3
    // --------------

    // TODO 1.: Replique o código do método LoadTexture da classe Game, mas desta vez carregue a imagem
    //  a partir do caminho imagePath passado como parâmetro. Arma zene o resultado em mTexture.
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
    // --------------
    // TODO - PARTE 1-3
    // --------------

    // TODO 1.: Verifique se mTexture é nula. Se for, retorne imediatamente.
    if (!mTexture || !mIsVisible) {
        return;
    }

    // TODO 2.: Crie um SDL_Rect para definir a posição e o tamanho da imagem na tela. A posição deve ser
    //  relativa ao screenPos passado como parâmetro, ou seja, some screenPos com mPosition.
    SDL_Rect src = {static_cast<int>(screenPos.x + mPosition.x),
                         static_cast<int>(screenPos.y + mPosition.y),
                         static_cast<int>(mSize.x),
                         static_cast<int>(mSize.y)};

    // TODO 3.: Desenhe a textura mTexture no renderer usando SDL_RenderCopy.
    SDL_RenderCopyEx(renderer,mTexture,NULL,&src,0.0,NULL,SDL_FLIP_NONE);
}