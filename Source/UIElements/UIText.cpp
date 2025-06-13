//
// Created by Lucas N. Ferreira on 22/05/25.
//

#include "UIText.h"
#include "UIFont.h"

UIText::UIText(const std::string &text, class UIFont* font, int pointSize, const unsigned wrapLength,
               const Vector2 &pos, const Vector2 &size, const Vector3 &color)
   :UIElement(pos, size, color)
   ,mFont(font)
   ,mPointSize(pointSize)
   ,mWrapLength(wrapLength)
   ,mTextTexture(nullptr)
{
    // --------------
    // TODO - PARTE 1-1
    // --------------

    // TODO 1.: Utilize o método SetText para definir o texto inicial do UIText.
    SetText(text);
}

UIText::~UIText()
{

}

void UIText::SetText(const std::string &text) {
    // --------------
    // TODO - PARTE 1-1
    // --------------

    // TODO 1.: Verifique se a textura atual mTextTexture já foi inicializada. Se sim, destrua-a com SDL_DestroyTexture
    //  e defina mTextTexture como nullptr. Caso contrário, siga para o próximo passo.
    if (mTextTexture) {
        SDL_DestroyTexture(mTextTexture);
        mTextTexture = nullptr;
    }

    // TODO 2.: Crie a textura de texto usando o método RenderText do mFont, passando o texto, a cor, o tamanho do
    //  ponto e o comprimento de quebra. Armazene a textura resultante em mTextTexture e a nova string em mText.
    mTextTexture = mFont->RenderText(text,mColor,mPointSize,mWrapLength);
    mText = text;
}

void UIText::Draw(SDL_Renderer *renderer, const Vector2 &screenPos)
{
    // --------------
    // TODO - PARTE 1-1
    // --------------

    // TODO 1.: Crie um SDL_Rect chamado titleQuad que representa a posição e o tamanho do texto na tela. Como elementos
    //  de UI geralmente são desenhados usando posição relativa, e não absoluta, some a posição do elemento UI (mPosition)
    //  com a posição da tela (screenPos) para obter a posição final do texto.

    SDL_Rect titleQuad = {
        static_cast<int>(mPosition.x +screenPos.x),
        static_cast<int>(mPosition.y +screenPos.y),
        static_cast<int>(mSize.x),
        static_cast<int>(mSize.y)
    };

    // TODO 2.: Desenhe a textura de texto mTextTexture usando SDL_RenderCopyEx. Use o renderer passado como parâmetro.
    SDL_RenderCopyEx(renderer,mTextTexture,NULL,&titleQuad,0.0,NULL,SDL_FLIP_NONE);
}