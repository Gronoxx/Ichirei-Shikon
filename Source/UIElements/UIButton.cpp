//
// Created by Lucas N. Ferreira on 22/05/25.
//

#include "UIButton.h"

#include "UIImage.h"
#include "UIScreen.h"
#include "../Game.h"

UIButton::UIButton(UIScreen * MainMenu, const std::string& text, class UIFont* font, std::function<void()> onClick,
                   const Vector2& pos, const Vector2& size, Game* game,bool isSizeDynamic, const Vector3& color,
                   int pointSize , unsigned wrapLength,
                   const Vector2 &textPos, const Vector2 &textSize, const Vector3& textColor)
        :UIElement(pos, size, color)
        ,mOnClick(onClick)
        ,mHighlighted(false)
        ,mText(text, font, pointSize, wrapLength, textPos, textSize, textColor)
        ,mGame(game)
        ,mMainMenu(MainMenu)
        ,mIsSizeDynamic(isSizeDynamic)
{

    //TODO CUSTOMIZACAO DEFINIR O TAMANHO DO BOTAO DE ACORDO COM O TAMANHO DO TEXTO
    if (mIsSizeDynamic) {
        int width, height;
        SDL_QueryTexture(mText.GetTextTexture(), NULL, NULL, &width, &height);
        mSize.x = static_cast<float>(width);
        mSize.y = static_cast<float>(height);
        mText.SetSize(Vector2(width, height));
    }

}

UIButton::~UIButton()
{

}


void UIButton::Draw(SDL_Renderer *renderer, const Vector2 &screenPos)
{
    // --------------
    // TODO - PARTE 1-2
    // --------------


    // TODO 1.: Crie um SDL_Rect chamado titleQuad com a posição relativa do botão na tela. Some a posição do botão
    //  (mPosition) com a posição da tela (screenPos) para obter a posição final do botão.
    //  Use mSize para definir a largura e altura.
    SDL_Rect titleQuad = {static_cast<int>(screenPos.x + mPosition.x),
                          static_cast<int>(screenPos.y + mPosition.y),
                          static_cast<int>(mSize.x),
                          static_cast<int>(mSize.y)};

    // TODO 2.: Verifique se o botão está destacado (mHighlighted). Se sim, defina a cor de preenchimento do
    //  retângulo como laranja (200, 100, 0, 255) usando SDL_SetRenderDrawColor. Em seguida,
    //  desenhe o retângulo usando SDL_RenderFillRect com o renderer passado como parâmetro.
    if (mHighlighted) {
        if (mIcon == nullptr){
            Vector2 dims (24.0f,24.0f);
            Vector2 pos (screenPos.x + mPosition.x - dims.x - iconPadding, screenPos.y + mPosition.y);
            mIcon = mMainMenu->AddImage("../Assets/Sprites/Collectables/HUDMushroom.png", pos, dims);
        }
        else if (mIcon->GetVisible() == false) {
            mIcon->SetVisible(true);
        }
    }
    else {
        if (mIcon != nullptr) {
            mIcon->SetVisible(false);
        }
    }

    // TODO 3.: Desenhe o texto do botão usando o método Draw da classe UIText. Use posição relativa ao botão, ou seja,
    //  a posição do texto deve ser o centro do botão menos a metade do tamanho do texto.
    Vector2 textPos = screenPos + mPosition + (mSize * 0.5) - (mText.GetSize() * 0.5);
    mText.Draw(renderer, textPos);
}

void UIButton::OnClick()
{
    // --------------
    // TODO - PARTE 1-2
    // --------------

    // TODO 1.: Verifique se o ponteiro mOnClick não é nulo. Se não for, chame a função mOnClick().
    if (mOnClick) {
        mOnClick();
    }
}