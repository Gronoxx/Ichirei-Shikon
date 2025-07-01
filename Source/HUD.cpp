//
// Created by Lucas N. Ferreira on 08/12/23.
//

#include "HUD.h"
#include "Game.h"
#include "UIElements/UIText.h"
#include "UIElements/UIRect.h"
#include "UIElements/UITimerBar.h"



HUD::HUD(class Game* game, const std::string& fontName, SDL_Renderer* renderer)
    : UIScreen(game, fontName) {
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
    int barWidth = (barArea.w - (maxBars + 1) * padding) / maxBars;
    int barHeight = barArea.h - 2 * padding;

    for (int i = 0; i < currentBars; ++i)
    {
        float x = barArea.x + padding + i * (barWidth + padding);
        float y = barArea.y + padding;

        // Azul principal
        auto* bar = AddRect(Vector2(x, y), Vector2(barWidth, barHeight), Vector3(91, 187, 255));
        mBatteryBars.push_back(bar);

        AddRect(Vector2(x, y), Vector2(barWidth, barHeight / 3), Vector3(200, 240, 255));
    }

    AddTimerBar(Vector2(mGame->GetWindowWidth() / 2 - TIMER_WIDTH / 2 + 18, 3),
            Vector2(93, 8), 60.0f);
}

HUD::~HUD()
{

}

void HUD::SetTime(int time)
{
    std::string timeStr = std::to_string(time);

    int numDigits = timeStr.length();
    float newWidth = numDigits * CHAR_WIDTH;
    Vector2 newSize(newWidth, WORD_HEIGHT);

    float newPosX = mGame->GetWindowWidth() - WORD_OFFSET - (numDigits * CHAR_WIDTH);

    float newPosY = HUD_POS_Y * 2.0f + WORD_HEIGHT;
    Vector2 newPosition(newPosX, newPosY);

    // 4. Atualizar o objeto de texto com os novos valores.

}

void HUD::SetLevelName(const std::string &levelName)
{

}

void HUD::SetCoins()
{
    std::string coinsStr = std::to_string(mGame->GetNumberOfCoinsCollected());


}

void HUD::SetScore()
{
    int score = mGame->GetScore();
    std::stringstream ss;
    ss << std::setw(6) << std::setfill('0') << score;

    std::string coinsStr = ss.str();


}

void HUD::TakeDamage()
{
    if (!mBatteryBars.empty())
    {
        auto* lastBar = mBatteryBars.back();
        mBatteryBars.pop_back();

        // Remove também da lista interna de retângulos
        auto it = std::find(mRects.begin(), mRects.end(), lastBar);
        if (it != mRects.end())
            mRects.erase(it); // Remove o ponteiro da lista

        delete lastBar; // Agora sim é seguro deletar
    }

    // Atualiza contador lógico
    currentBars--;

    if (currentBars <= 0)
    {
        // Se necessário, pode logar algo
        SDL_Log("Player está sem energia!");
    }
}

void HUD::Update(float deltaTime) {
    if(mTimerBar)
        mTimerBar->Update(deltaTime);
}
