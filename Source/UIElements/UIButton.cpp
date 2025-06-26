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
    // Defina o padding desejado
    const int paddingX = 16;
    const int paddingY = 8;

    Vector2 textSize = mText.GetSize();
    Vector2 rectSize = textSize + Vector2(paddingX * 2, paddingY * 2);

    SDL_Rect titleQuad = {static_cast<int>(screenPos.x + mPosition.x),
                          static_cast<int>(screenPos.y + mPosition.y),
                          static_cast<int>(rectSize.x),
                          static_cast<int>(rectSize.y)};

    if (mHighlighted) {
        Uint32 ticks = SDL_GetTicks();
        float phase = static_cast<float>((ticks % 1200)) / 1200.0f;
        float alpha = std::sin(phase * 3.14159265f);
        Uint8 a = static_cast<Uint8>(alpha * 255.0f);
        SDL_SetRenderDrawColor(renderer, 51, 7, 5, a);
    } else {
        SDL_SetRenderDrawColor(renderer, static_cast<Uint8>(mColor.x * 255),
                                           static_cast<Uint8>(mColor.y * 255),
                                           static_cast<Uint8>(mColor.z * 255), 0);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_RenderFillRect(renderer, &titleQuad);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    Vector2 textPos = Vector2(titleQuad.x, titleQuad.y) + Vector2(paddingX, paddingY);
    mText.Draw(renderer, textPos);
}

void UIButton::OnClick()
{
    if (mOnClick) {
        mOnClick();
    }
}