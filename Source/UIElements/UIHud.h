#pragma once

#include <string>
#include "UIScreen.h"
#include <iomanip>
#include <vector>
#include "UITimerBar.h"

class UIRect;

class UIHud final : public UIScreen
{
public:
    const int POINT_SIZE = 48;
    const int WORD_HEIGHT = 20.0f;
    const int WORD_OFFSET = 25.0f;
    const int CHAR_WIDTH = 20.0f;
    const int HUD_POS_Y = 10.0f;

    UIHud(Game* game, const std::string& fontName, SDL_Renderer * renderer);
    ~UIHud() override;

    void TakeDamage();
    int GetCurrentBattery() const { return currentBars; }
    void Update(float deltaTime) override;
    void Draw(SDL_Renderer *renderer) override;

    // Create TimeBar
    UITimerBar* AddTimerBar(const Vector2& pos, const Vector2& size, float duration);

private:
    const int HUD_HEIGHT = 23;
    const int BATTERY_WIDTH = 77;
    const int BATTERY_HEIGHT = 19;
    const int TIMER_WIDTH = 112;
    const int TIMER_HEIGHT = 19;
    const int SUBWEAPON_WIDTH =56;
    const int SUPWEAPON_HEIGHT = 24;
    const int MAX_BARS = 11;

    float scaleX = static_cast<float>(mGame->GetWindowWidth()) / 640.0f;
    int currentBars = 11;
    SDL_Rect barArea = { 10, 4, BATTERY_WIDTH - 21, BATTERY_HEIGHT - 5 };

    std::vector<UIRect*> mBatteryBars;
    UIImage* mBaseHUDImage;
    UIImage* mBatteryImage;
    UIImage * mTimerImage;
    UIImage* mSubWeaponImage;
    UITimerBar* mTimerBar;
};
