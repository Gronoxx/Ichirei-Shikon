#include "UIHud.h"
#include "../Game.h"
#include "UIText.h"
#include "UIRect.h"
#include "UITimerBar.h"

UIHud::UIHud(Game* game, const std::string& fontName, SDL_Renderer*)
    : UIScreen(game, fontName),
        mTimerBar(nullptr){
    // Imagens base
    mBaseHUDImage = AddImage("Assets/Sprites/HUD/spr_hud.png", Vector2(0.0f, 0.0f),
                             Vector2(mGame->GetWindowWidth(), HUD_HEIGHT));

    mBatteryImage = AddImage("Assets/Sprites/HUD/spr_hud_battery.png", Vector2(0.0f, 0.0f),
                             Vector2(BATTERY_WIDTH * scaleX, BATTERY_HEIGHT));

    mTimerImage = AddImage("Assets/Sprites/HUD/spr_hud_timer.png", Vector2(mGame->GetWindowWidth() / 2 - TIMER_WIDTH / 2, 0.0f),
                           Vector2(TIMER_WIDTH * scaleX, TIMER_HEIGHT));

    mSubWeaponImage = AddImage("Assets/Sprites/HUD/spr_hud_subweapon.png",
                               Vector2(mGame->GetWindowWidth() - SUBWEAPON_WIDTH * scaleX, 0.0f),
                               Vector2(SUBWEAPON_WIDTH * scaleX, SUPWEAPON_HEIGHT));

    // Barras internas da bateria (preenchidas com azul e brilho)
    int padding = 1;
    int barWidth = (barArea.w - (MAX_BARS + 1) * padding) / MAX_BARS;
    int barHeight = barArea.h - 2 * padding;

    for (int i = 0; i < currentBars; ++i)
    {
        float x = barArea.x + padding + i * (barWidth + padding);
        float y = barArea.y + padding;

        // Azul principal
        auto* bar = AddRect(Vector2(x, y), Vector2(barWidth, barHeight), Vector4(91, 187, 255, 255));
        mBatteryBars.push_back(bar);

        AddRect(Vector2(x, y), Vector2(barWidth, barHeight / 3), Vector4(200, 240, 255, 255));
    }

    AddTimerBar(Vector2(mGame->GetWindowWidth() / 2 - TIMER_WIDTH / 2 + 18, 3),
            Vector2(93, 8), 60.0f);
}

UIHud::~UIHud()
{
    if (mTimerBar) {
        delete mTimerBar;
        mTimerBar = nullptr;
    }
}

void UIHud::TakeDamage()
{
    if (!mBatteryBars.empty())
    {
        const auto* lastBar = mBatteryBars.back();
        mBatteryBars.pop_back();

        // Remove também da lista interna de retângulos
        if (const auto it = std::find(mRects.begin(), mRects.end(), lastBar); it != mRects.end())
            mRects.erase(it); // Remove o ponteiro da lista

        delete lastBar;
    }

    // Atualiza contador lógico
    currentBars--;
}

void UIHud::Update(const float deltaTime) {
    if(mTimerBar)
        mTimerBar->Update(deltaTime);
}

void UIHud::Draw(SDL_Renderer *renderer) {
    for (int i=0;i<mTexts.size();i++)
        mTexts[i]->Draw(renderer,mPos);

    for (int i=0;i<mButtons.size();i++)
        mButtons[i]->Draw(renderer,mPos);

    for (int i=0;i<mImages.size();i++)
        mImages[i]->Draw(renderer,mPos);

    for (int i=0;i<mRects.size();i++) {
        if (mRects[i])
            mRects[i]->Draw(renderer,mPos);
    }

    if(mTimerBar)
        mTimerBar->Draw(renderer,mPos);
}

UITimerBar* UIHud::AddTimerBar(const Vector2& pos, const Vector2& size, const float duration)
{
    if (mTimerBar)
    {
        delete mTimerBar; // Limpa o anterior, se houver
        mTimerBar = nullptr;
    }

    mTimerBar = new UITimerBar(pos, size, duration);
    return mTimerBar;
}
